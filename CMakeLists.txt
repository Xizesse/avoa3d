cmake_minimum_required(VERSION 3.5)
project(avoa3d)

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(nav_msgs REQUIRED) 
find_package(tf2_ros REQUIRED)

# Generate custom messages
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/ElementCharacteristicsStamped.msg"
  LIBRARY_NAME ${PROJECT_NAME}
  DEPENDENCIES std_msgs geometry_msgs builtin_interfaces
)

# Include directories
include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_cpp
)

# Get the typesupport target
rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")

# Create executables
add_executable(rviz_marker src/rviz_marker.cpp)
add_executable(point_cloud_publisher src/point_cloud_publisher.cpp)
add_executable(velocity_publisher src/velocity_publisher.cpp)
add_executable(obstacle_publisher src/obstacle_publisher.cpp)
add_executable(test_w_goal src/test_w_goal.cpp)

# Link dependencies to executables
ament_target_dependencies(rviz_marker
  rclcpp
  visualization_msgs
  geometry_msgs
  std_msgs
  nav_msgs
  tf2_ros
)

ament_target_dependencies(point_cloud_publisher
  rclcpp
  sensor_msgs
)

ament_target_dependencies(velocity_publisher
  rclcpp
  geometry_msgs
  nav_msgs
  tf2_ros
)

ament_target_dependencies(test_w_goal
  rclcpp
  geometry_msgs
  nav_msgs
  tf2_ros
)

ament_target_dependencies(obstacle_publisher
  rclcpp
  std_msgs
  geometry_msgs
)

# Link the typesupport target to the executables
target_link_libraries(rviz_marker ${cpp_typesupport_target})
target_link_libraries(point_cloud_publisher ${cpp_typesupport_target})
target_link_libraries(velocity_publisher ${cpp_typesupport_target})
target_link_libraries(obstacle_publisher ${cpp_typesupport_target})
target_link_libraries(test_w_goal ${cpp_typesupport_target})

# Install executables
install(TARGETS
  rviz_marker
  point_cloud_publisher
  velocity_publisher
  test_w_goal
  obstacle_publisher
  DESTINATION lib/${PROJECT_NAME})

# Enable testing
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

# Install launch files.
install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}/
)

# Ament package
ament_package()
