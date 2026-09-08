#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, QoSReliabilityPolicy, QoSDurabilityPolicy, QoSHistoryPolicy
from nav_msgs.msg import Odometry
from geographic_msgs.msg import GeoPointStamped
from avoa3d.msg import ElementCharacteristicsArray, ElementCharacteristicsStamped
from visualization_msgs.msg import Marker, MarkerArray
import math

class NautilusObstaclePublisher(Node):
    def __init__(self):
        super().__init__('nautilus_obstacle_publisher')
        
        # Parameters
        self.declare_parameter('agent_odom_topic', '/usv/lily/mavros/local_position/odom')
        self.declare_parameter('agent_origin_topic', '/usv/lily/mavros/global_position/gp_origin')
        
        self.declare_parameter('nautilus_odom_topic', '/usv/nautilus/mavros/local_position/odom')
        self.declare_parameter('nautilus_origin_topic', '/usv/nautilus/mavros/global_position/gp_origin')
        
        self.declare_parameter('output_topic', '/element_tracking/elements')
        self.declare_parameter('marker_topic', '/element_tracking/markers')
        self.declare_parameter('nautilus_radius', 3.0)
        
        self.nautilus_radius = self.get_parameter('nautilus_radius').value
        
        # QoS Profiles
        odom_qos = QoSProfile(
            reliability=QoSReliabilityPolicy.BEST_EFFORT,
            durability=QoSDurabilityPolicy.VOLATILE,
            history=QoSHistoryPolicy.KEEP_LAST,
            depth=10
        )
        
        nautilus_gp_qos = QoSProfile(
            reliability=QoSReliabilityPolicy.RELIABLE,
            durability=QoSDurabilityPolicy.TRANSIENT_LOCAL,
            history=QoSHistoryPolicy.KEEP_LAST,
            depth=10
        )
        
        agent_gp_qos = QoSProfile(
            reliability=QoSReliabilityPolicy.RELIABLE,
            durability=QoSDurabilityPolicy.VOLATILE,
            history=QoSHistoryPolicy.KEEP_LAST,
            depth=10
        )
        
        # Subscriptions
        self.agent_odom_sub = self.create_subscription(Odometry, self.get_parameter('agent_odom_topic').value, self.agent_odom_callback, odom_qos)
        self.agent_origin_sub = self.create_subscription(GeoPointStamped, self.get_parameter('agent_origin_topic').value, self.agent_origin_callback, agent_gp_qos)
        
        self.nautilus_odom_sub = self.create_subscription(Odometry, self.get_parameter('nautilus_odom_topic').value, self.nautilus_odom_callback, odom_qos)
        self.nautilus_origin_sub = self.create_subscription(GeoPointStamped, self.get_parameter('nautilus_origin_topic').value, self.nautilus_origin_callback, nautilus_gp_qos)
            
        # Publishers
        self.publisher = self.create_publisher(ElementCharacteristicsArray, self.get_parameter('output_topic').value, 10)
        self.marker_pub = self.create_publisher(MarkerArray, self.get_parameter('marker_topic').value, 10)
            
        # Data storage
        self.agent_pos = None
        self.agent_yaw = None
        self.agent_gp_origin = None # (lat, lon)
        
        self.nautilus_gp_origin = None # (lat, lon)
        self.nautilus_pos = None
        self.nautilus_vel = None
        self.nautilus_yaw = None
        
        self.offset_enu = None # (x, y) offset from agent origin to nautilus origin
        
        # Timer for publishing at a fixed rate
        self.timer = self.create_timer(0.1, self.publish_obstacles) # 10Hz

        self.get_logger().info("Nautilus Obstacle Publisher initialized.")

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

    def rotate_2d(self, x, y, angle):
        """Rotate a 2D vector by an angle."""
        cos_a = math.cos(angle)
        sin_a = math.sin(angle)
        return x * cos_a - y * sin_a, x * sin_a + y * cos_a

    def agent_odom_callback(self, msg):
        self.agent_pos = msg.pose.pose.position
        self.agent_yaw = self.get_yaw(msg.pose.pose.orientation)

    def agent_origin_callback(self, msg):
        if self.agent_gp_origin is None:
            self.agent_gp_origin = (msg.position.latitude, msg.position.longitude)
            self.get_logger().info(f"Agent GP Origin received: {self.agent_gp_origin}")
            self.calculate_offset()

    def nautilus_odom_callback(self, msg):
        self.nautilus_pos = msg.pose.pose.position
        self.nautilus_vel = msg.twist.twist.linear
        self.nautilus_yaw = self.get_yaw(msg.pose.pose.orientation)

    def nautilus_origin_callback(self, msg):
        if self.nautilus_gp_origin is None:
            self.nautilus_gp_origin = (msg.position.latitude, msg.position.longitude)
            self.get_logger().info(f"Nautilus GP Origin received: {self.nautilus_gp_origin}")
            self.calculate_offset()

    def calculate_offset(self):
        if self.agent_gp_origin is not None and self.nautilus_gp_origin is not None:
            # Calculate offset of nautilus origin relative to agent origin in ENU
            x, y = self.gps_to_enu(self.nautilus_gp_origin[0], self.nautilus_gp_origin[1], 
                                   self.agent_gp_origin[0], self.agent_gp_origin[1])
            self.offset_enu = (x, y)
            self.get_logger().info(f"Nautilus origin offset (ENU) calculated: [{x:.2f}, {y:.2f}]")

    def publish_obstacles(self):
        missing = []
        if self.agent_pos is None: missing.append("agent_pos")
        if self.agent_yaw is None: missing.append("agent_yaw")
        if self.offset_enu is None: missing.append("offset_enu")
        if self.nautilus_pos is None: missing.append("nautilus_pos")
        if self.nautilus_vel is None: missing.append("nautilus_vel")
        if self.nautilus_yaw is None: missing.append("nautilus_yaw")
        
        if missing:
            # Throttle print to avoid spam
            if not hasattr(self, 'last_print_time') or (self.get_clock().now().nanoseconds - self.last_print_time) > 1e9:
                self.get_logger().info(f"Missing data: {missing}")
                self.last_print_time = self.get_clock().now().nanoseconds
            return
            
        msg_array = ElementCharacteristicsArray()
        marker_array = MarkerArray()
        
        # 1. Position
        # Nautilus ENU in agent world frame
        nautilus_world_x = self.nautilus_pos.x + self.offset_enu[0]
        nautilus_world_y = self.nautilus_pos.y + self.offset_enu[1]
        
        # World ENU -> USV base_link
        dx = nautilus_world_x - self.agent_pos.x
        dy = nautilus_world_y - self.agent_pos.y
        rel_x, rel_y = self.rotate_2d(dx, dy, -self.agent_yaw)
        
        # 2. Velocity
        # Assuming MAVROS odometry velocity is in local frame (body frame base_link)
        # Transform Nautilus Body Vel -> Agent World ENU
        vw_x, vw_y = self.rotate_2d(self.nautilus_vel.x, self.nautilus_vel.y, self.nautilus_yaw)
        
        # World ENU -> USV base_link
        v_rel_x, v_rel_y = self.rotate_2d(vw_x, vw_y, -self.agent_yaw)
        
        # 3. Fill AVOA Element
        element = ElementCharacteristicsStamped()
        element.header.stamp = self.get_clock().now().to_msg()
        element.header.frame_id = 'base_link'
        element.pose.position.x = rel_x
        element.pose.position.y = rel_y
        element.velocity.x = v_rel_x
        element.velocity.y = v_rel_y
        
        radius = self.nautilus_radius
        element.size.x = radius * 2.0
        element.size.y = radius * 2.0
        element.size.z = radius * 2.0
        msg_array.elements.append(element)
        
        # 4. Markers
        # Cylinder (Radius)
        m_cyl = Marker()
        m_cyl.header = element.header
        m_cyl.ns = "nautilus_radius"
        m_cyl.id = 10
        m_cyl.type = Marker.CYLINDER
        m_cyl.pose.position.x, m_cyl.pose.position.y = rel_x, rel_y
        m_cyl.scale.x = m_cyl.scale.y = radius * 2.0
        m_cyl.scale.z = 0.1
        m_cyl.color.r, m_cyl.color.g, m_cyl.color.b = (0.0, 1.0, 0.0) # Green
        m_cyl.color.a = 0.7
        marker_array.markers.append(m_cyl)
        
        # Arrow (Velocity)
        m_arr = Marker()
        m_arr.header = element.header
        m_arr.ns = "nautilus_velocity"
        m_arr.id = 11
        m_arr.type = Marker.ARROW
        m_arr.pose.position.x, m_arr.pose.position.y, m_arr.pose.position.z = rel_x, rel_y, 0.5
        
        speed = math.sqrt(v_rel_x**2 + v_rel_y**2)
        if speed > 0.05:
            v_yaw = math.atan2(v_rel_y, v_rel_x)
            m_arr.pose.orientation.z = math.sin(v_yaw / 2.0)
            m_arr.pose.orientation.w = math.cos(v_yaw / 2.0)
            m_arr.scale.x = max(1.0, speed * 2.0) # Length based on speed
        else:
            m_arr.scale.x = 0.001 # Hide if not moving
        m_arr.scale.y = m_arr.scale.z = 0.2
        m_arr.color.r = m_arr.color.g = m_arr.color.b = 1.0
        m_arr.color.a = 1.0
        marker_array.markers.append(m_arr)
        
        self.publisher.publish(msg_array)
        self.marker_pub.publish(marker_array)

def main(args=None):
    rclpy.init(args=args)
    node = NautilusObstaclePublisher()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
