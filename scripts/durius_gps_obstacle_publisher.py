#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry
from geographic_msgs.msg import GeoPointStamped
from avoa3d.msg import ElementCharacteristicsArray, ElementCharacteristicsStamped
from visualization_msgs.msg import Marker, MarkerArray
import math

class DuriusGpsObstaclePublisher(Node):
    def __init__(self):
        super().__init__('durius_gps_obstacle_publisher')
        
        # --- EDIT COORDINATES HERE ---
        # Default: Durius GPS
        default_lat = 41.685703
        default_lon = -8.838605
        # -----------------------------

        # Parameters
        self.declare_parameter('durius_lat', default_lat)
        self.declare_parameter('durius_lon', default_lon)
        self.declare_parameter('durius_radius', 10.0)
        self.declare_parameter('agent_odom_topic', '/usv/lily/mavros/local_position/odom')
        self.declare_parameter('origin_topic', '/usv/lily/mavros/global_position/gp_origin')
        self.declare_parameter('output_topic', '/element_tracking/elements')
        self.declare_parameter('marker_topic', '/element_tracking/markers')
        
        self.durius_lat = self.get_parameter('durius_lat').value
        self.durius_lon = self.get_parameter('durius_lon').value
        self.durius_radius = self.get_parameter('durius_radius').value
        
        # Subscriptions
        self.agent_sub = self.create_subscription(Odometry, self.get_parameter('agent_odom_topic').value, self.agent_callback, 10)
        self.origin_sub = self.create_subscription(GeoPointStamped, self.get_parameter('origin_topic').value, self.origin_callback, 10)
            
        # Publishers
        self.publisher = self.create_publisher(ElementCharacteristicsArray, self.get_parameter('output_topic').value, 10)
        self.marker_pub = self.create_publisher(MarkerArray, self.get_parameter('marker_topic').value, 10)
            
        # Data storage
        self.agent_pos = None
        self.agent_yaw = None
        self.datum_lat = None
        self.datum_lon = None
        self.durius_enu = None
        
        # Timer for publishing at a fixed rate
        self.timer = self.create_timer(0.1, self.publish_obstacles) # 10Hz

        self.get_logger().info(f"Durius GPS Obstacle Publisher initialized. GPS: {self.durius_lat}, {self.durius_lon}")

    def get_yaw(self, q):
        """Extract yaw from quaternion."""
        return math.atan2(2.0 * (q.w * q.z + q.x * q.y), 1.0 - 2.0 * (q.y * q.y + q.z * q.z))

    def gps_to_enu(self, lat, lon, lat0, lon0):
        """Convert GPS to local ENU."""
        deg2rad = math.pi / 180.0
        scale = 111319.5
        lat0_rad = lat0 * deg2rad
        return (
            (lon - lon0) * math.cos(lat0_rad) * scale,
            (lat - lat0) * scale
        )

    def agent_callback(self, msg):
        self.agent_pos = msg.pose.pose.position
        self.agent_yaw = self.get_yaw(msg.pose.pose.orientation)

    def origin_callback(self, msg):
        if self.datum_lat is not None:
            return
        self.datum_lat = msg.position.latitude
        self.datum_lon = msg.position.longitude
        
        # Calculate Durius ENU once
        x, y = self.gps_to_enu(self.durius_lat, self.durius_lon, self.datum_lat, self.datum_lon)
        self.durius_enu = (x, y)
        self.get_logger().info(f"Durius ENU calculated: [{x:.2f}, {y:.2f}]")

    def rotate_2d(self, x, y, angle):
        """Rotate a 2D vector by an angle."""
        cos_a = math.cos(angle)
        sin_a = math.sin(angle)
        return x * cos_a - y * sin_a, x * sin_a + y * cos_a

    def publish_obstacles(self):
        if self.agent_pos is None or self.agent_yaw is None or self.durius_enu is None:
            return
            
        msg_array = ElementCharacteristicsArray()
        marker_array = MarkerArray()
        
        # 1. Position: World ENU -> USV base_link
        dx = self.durius_enu[0] - self.agent_pos.x
        dy = self.durius_enu[1] - self.agent_pos.y
        rel_x, rel_y = self.rotate_2d(dx, dy, -self.agent_yaw)
        
        # 2. Fill AVOA Element
        element = ElementCharacteristicsStamped()
        element.header.stamp = self.get_clock().now().to_msg()
        element.header.frame_id = 'base_link'
        element.pose.position.x = rel_x
        element.pose.position.y = rel_y
        element.velocity.x = 0.0 # Static
        element.velocity.y = 0.0
        radius = self.durius_radius
        element.size.x = radius * 2.0
        element.size.y = radius * 2.0
        element.size.z = radius * 2.0
        msg_array.elements.append(element)
        
        # 3. Marker
        m_cyl = Marker()
        m_cyl.header = element.header
        m_cyl.ns = "durius_static"
        m_cyl.id = 100
        m_cyl.type = Marker.CYLINDER
        m_cyl.pose.position.x, m_cyl.pose.position.y = rel_x, rel_y
        m_cyl.scale.x = m_cyl.scale.y = radius * 2.0
        m_cyl.scale.z = 0.1
        m_cyl.color.r, m_cyl.color.g, m_cyl.color.b = (1.0, 0.0, 0.0) # Red
        m_cyl.color.a = 0.4
        marker_array.markers.append(m_cyl)
        
        self.publisher.publish(msg_array)
        self.marker_pub.publish(marker_array)

def main(args=None):
    rclpy.init(args=args)
    node = DuriusGpsObstaclePublisher()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
