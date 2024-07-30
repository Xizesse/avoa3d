cmake_minimum_required(VERSION 3.5)
project(avoa)

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(rosidl_default_generators REQUIRED)

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
add_executable(a_publisher src/a_publisher.cpp)
add_executable(point_cloud_publisher src/point_cloud_publisher.cpp)
add_executable(velocity_publisher src/velocity_publisher.cpp)
add_executable(obstacle_publisher src/obstacle_publisher.cpp)

# Link dependencies to executables
ament_target_dependencies(a_publisher
  rclcpp
  visualization_msgs
)

ament_target_dependencies(point_cloud_publisher
  rclcpp
  sensor_msgs
)

ament_target_dependencies(velocity_publisher
  rclcpp
  geometry_msgs
)

ament_target_dependencies(obstacle_publisher
  rclcpp
  std_msgs
  geometry_msgs
)

# Link the typesupport target to the executables
target_link_libraries(a_publisher ${cpp_typesupport_target})
target_link_libraries(point_cloud_publisher ${cpp_typesupport_target})
target_link_libraries(velocity_publisher ${cpp_typesupport_target})
target_link_libraries(obstacle_publisher ${cpp_typesupport_target})

# Install executables
install(TARGETS
  a_publisher
  point_cloud_publisher
  velocity_publisher
  obstacle_publisher
  DESTINATION lib/${PROJECT_NAME})

# Enable testing
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

# Ament package
ament_package()
