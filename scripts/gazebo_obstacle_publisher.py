#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry
from avoa3d.msg import ElementCharacteristicsArray, ElementCharacteristicsStamped
from visualization_msgs.msg import Marker, MarkerArray
import math

class GazeboObstaclePublisher(Node):
    def __init__(self):
        super().__init__('gazebo_obstacle_publisher')
        
        # Parameters
        self.declare_parameter('agent_odom_topic', '/usv/lily/mavros/local_position/odom')
        self.declare_parameter('durius_odom_topic', '/model/DURIUS/odometry')
        self.declare_parameter('nautilus_odom_topic', '/model/nautilus/odometry')
        self.declare_parameter('buoy_south_odom_topic', '/model/buoy_south_30m/odometry')
        self.declare_parameter('buoy_west_odom_topic', '/model/buoy_west_20m/odometry')
        self.declare_parameter('nautilys_north_odom_topic', '/model/nautilys_north_40m/odometry')
        
        self.declare_parameter('output_topic', '/element_tracking/elements')
        self.declare_parameter('marker_topic', '/element_tracking/markers')
        self.declare_parameter('durius_radius', 10.0)
        self.declare_parameter('nautilus_radius', 2.0)
        self.declare_parameter('obstacle_radius', 0.5)
        
        # Subscriptions
        self.agent_sub = self.create_subscription(Odometry, self.get_parameter('agent_odom_topic').value, self.agent_callback, 10)
        self.durius_sub = self.create_subscription(Odometry, self.get_parameter('durius_odom_topic').value, self.durius_callback, 10)
        self.nautilus_sub = self.create_subscription(Odometry, self.get_parameter('nautilus_odom_topic').value, self.nautilus_callback, 10)
        self.buoy_south_sub = self.create_subscription(Odometry, self.get_parameter('buoy_south_odom_topic').value, self.buoy_south_callback, 10)
        self.buoy_west_sub = self.create_subscription(Odometry, self.get_parameter('buoy_west_odom_topic').value, self.buoy_west_callback, 10)
        self.nautilys_north_sub = self.create_subscription(Odometry, self.get_parameter('nautilys_north_odom_topic').value, self.nautilys_north_callback, 10)
            
        # Publishers
        self.publisher = self.create_publisher(ElementCharacteristicsArray, self.get_parameter('output_topic').value, 10)
        self.marker_pub = self.create_publisher(MarkerArray, self.get_parameter('marker_topic').value, 10)
            
        # Data storage
        self.agent_pos = None
        self.agent_yaw = None
        
        # We store the full msg or extracted info for each obstacle
        self.obstacles_data = {
            'durius': {'pos': None, 'vel_local': None, 'yaw': 0.0},
            'nautilus': {'pos': None, 'vel_local': None, 'yaw': 0.0},
            'buoy_south': {'pos': None, 'vel_local': None, 'yaw': 0.0},
            'buoy_west': {'pos': None, 'vel_local': None, 'yaw': 0.0},
            'nautilys_north': {'pos': None, 'vel_local': None, 'yaw': 0.0}
        }
        
        # Timer for publishing at a fixed rate
        self.timer = self.create_timer(0.1, self.publish_obstacles) # 10Hz

    def get_yaw(self, q):
        """Extract yaw from quaternion."""
        return math.atan2(2.0 * (q.w * q.z + q.x * q.y), 1.0 - 2.0 * (q.y * q.y + q.z * q.z))

    def agent_callback(self, msg):
        self.agent_pos = msg.pose.pose.position
        self.agent_yaw = self.get_yaw(msg.pose.pose.orientation)

    def durius_callback(self, msg):
        self.obstacles_data['durius']['pos'] = msg.pose.pose.position
        self.obstacles_data['durius']['vel_local'] = msg.twist.twist.linear
        self.obstacles_data['durius']['yaw'] = self.get_yaw(msg.pose.pose.orientation)

    def nautilus_callback(self, msg):
        self.obstacles_data['nautilus']['pos'] = msg.pose.pose.position
        self.obstacles_data['nautilus']['vel_local'] = msg.twist.twist.linear
        self.obstacles_data['nautilus']['yaw'] = self.get_yaw(msg.pose.pose.orientation)

    def buoy_south_callback(self, msg):
        self.obstacles_data['buoy_south']['pos'] = msg.pose.pose.position
        self.obstacles_data['buoy_south']['vel_local'] = msg.twist.twist.linear
        self.obstacles_data['buoy_south']['yaw'] = self.get_yaw(msg.pose.pose.orientation)

    def buoy_west_callback(self, msg):
        self.obstacles_data['buoy_west']['pos'] = msg.pose.pose.position
        self.obstacles_data['buoy_west']['vel_local'] = msg.twist.twist.linear
        self.obstacles_data['buoy_west']['yaw'] = self.get_yaw(msg.pose.pose.orientation)

    def nautilys_north_callback(self, msg):
        self.obstacles_data['nautilys_north']['pos'] = msg.pose.pose.position
        self.obstacles_data['nautilys_north']['vel_local'] = msg.twist.twist.linear
        self.obstacles_data['nautilys_north']['yaw'] = self.get_yaw(msg.pose.pose.orientation)

    def rotate_2d(self, x, y, angle):
        """Rotate a 2D vector by an angle."""
        cos_a = math.cos(angle)
        sin_a = math.sin(angle)
        return x * cos_a - y * sin_a, x * sin_a + y * cos_a

    def publish_obstacles(self):
        if self.agent_pos is None or self.agent_yaw is None:
            return
            
        msg_array = ElementCharacteristicsArray()
        marker_array = MarkerArray()
        
        # Define obstacles to process
        targets = [
            {'name': 'durius', 'radius': self.get_parameter('durius_radius').value, 'color': (1.0, 0.0, 0.0), 'id': 0},
            {'name': 'nautilus', 'radius': self.get_parameter('nautilus_radius').value, 'color': (0.0, 1.0, 0.0), 'id': 10},
            {'name': 'buoy_south', 'radius': self.get_parameter('obstacle_radius').value, 'color': (1.0, 1.0, 0.0), 'id': 20},
            {'name': 'buoy_west', 'radius': self.get_parameter('obstacle_radius').value, 'color': (1.0, 0.5, 0.0), 'id': 30},
            {'name': 'nautilys_north', 'radius': self.get_parameter('obstacle_radius').value, 'color': (0.5, 0.0, 1.0), 'id': 40}
        ]

        for target in targets:
            data = self.obstacles_data[target['name']]
            if data['pos'] is None:
                continue
                
            # 1. Position: World -> USV base_link
            dx = data['pos'].x - self.agent_pos.x
            dy = data['pos'].y - self.agent_pos.y
            rel_x, rel_y = self.rotate_2d(dx, dy, -self.agent_yaw)
            
            # 2. Velocity:
            # First, transform from Obstacle Local -> World (if it's local, which it usually is in Gazebo Odometry)
            v_local = data['vel_local']
            if v_local:
                # Local -> World
                vw_x, vw_y = self.rotate_2d(v_local.x, v_local.y, data['yaw'])
                # World -> USV base_link
                v_rel_x, v_rel_y = self.rotate_2d(vw_x, vw_y, -self.agent_yaw)
            else:
                v_rel_x, v_rel_y = 0.0, 0.0

            # 3. Fill AVOA Element
            element = ElementCharacteristicsStamped()
            element.header.stamp = self.get_clock().now().to_msg()
            element.header.frame_id = 'base_link'
            element.pose.position.x = rel_x
            element.pose.position.y = rel_y
            element.velocity.x = v_rel_x
            element.velocity.y = v_rel_y
            radius = target['radius']
            element.size.x = radius * 2.0
            element.size.y = radius * 2.0
            element.size.z = radius * 2.0
            msg_array.elements.append(element)
            
            # 4. Markers
            # Cylinder (Radius)
            m_cyl = Marker()
            m_cyl.header = element.header
            m_cyl.ns = target['name'] + "_radius"
            m_cyl.id = target['id']
            m_cyl.type = Marker.CYLINDER
            m_cyl.pose.position.x, m_cyl.pose.position.y = rel_x, rel_y
            m_cyl.scale.x = m_cyl.scale.y = radius * 2.0
            m_cyl.scale.z = 0.1
            m_cyl.color.r, m_cyl.color.g, m_cyl.color.b = target['color']
            m_cyl.color.a = 0.4
            marker_array.markers.append(m_cyl)
            
            # Arrow (Velocity)
            m_arr = Marker()
            m_arr.header = element.header
            m_arr.ns = target['name'] + "_velocity"
            m_arr.id = target['id'] + 1
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

        if msg_array.elements:
            self.publisher.publish(msg_array)
            self.marker_pub.publish(marker_array)

def main(args=None):
    rclpy.init(args=args)
    node = GazeboObstaclePublisher()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
