
cmake_minimum_required(VERSION 3.5)
project(s3vo)

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(sensor_msgs REQUIRED)

# Generate custom messages
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/ElementCharacteristicsStamped.msg"
  "msg/ElementCharacteristicsArray.msg"
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
add_executable(s3vonode
  src/s3vonode.cpp
  src/sample_evaluator.cpp
  src/sample_visualizer.cpp
  src/sample_generator.cpp
  src/holonomic_sample_generator.cpp
  src/diff_drive_sample_generator.cpp
)

add_executable(rviz_marker src/rviz_marker.cpp)
add_executable(element_markers src/element_markers.cpp)
add_executable(tf_publisher src/tf_publisher.cpp)

# Link dependencies
ament_target_dependencies(s3vonode
  rclcpp
  std_msgs
  geometry_msgs
  nav_msgs
  sensor_msgs
)

ament_target_dependencies(rviz_marker
  rclcpp
  visualization_msgs
  geometry_msgs
  std_msgs
  nav_msgs
  tf2_ros
  tf2_eigen
)

ament_target_dependencies(element_markers
  rclcpp
  visualization_msgs
)

ament_target_dependencies(tf_publisher
  rclcpp
  nav_msgs
  tf2
  tf2_ros
  geometry_msgs
)

# Link typesupport
target_link_libraries(s3vonode ${cpp_typesupport_target})
target_link_libraries(rviz_marker ${cpp_typesupport_target})
target_link_libraries(element_markers ${cpp_typesupport_target})
target_link_libraries(tf_publisher ${cpp_typesupport_target})

# Install executables
install(TARGETS
  s3vonode
  rviz_marker
  element_markers
  tf_publisher
  DESTINATION lib/${PROJECT_NAME}
)

# Enable testing
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

# Install launch files and config
install(DIRECTORY
  launch
  config
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()
