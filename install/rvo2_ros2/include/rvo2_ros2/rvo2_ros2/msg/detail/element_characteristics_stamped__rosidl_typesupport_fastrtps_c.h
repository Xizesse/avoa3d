// generated from rosidl_typesupport_fastrtps_c/resource/idl__rosidl_typesupport_fastrtps_c.h.em
// with input from rvo2_ros2:msg/ElementCharacteristicsStamped.idl
// generated code does not contain a copyright notice
#ifndef RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__ROSIDL_TYPESUPPORT_FASTRTPS_C_H_
#define RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__ROSIDL_TYPESUPPORT_FASTRTPS_C_H_


#include <stddef.h>
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_interface/macros.h"
#include "rvo2_ros2/msg/rosidl_typesupport_fastrtps_c__visibility_control.h"
#include "rvo2_ros2/msg/detail/element_characteristics_stamped__struct.h"
#include "fastcdr/Cdr.h"

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_rvo2_ros2
bool cdr_serialize_rvo2_ros2__msg__ElementCharacteristicsStamped(
  const rvo2_ros2__msg__ElementCharacteristicsStamped * ros_message,
  eprosima::fastcdr::Cdr & cdr);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_rvo2_ros2
bool cdr_deserialize_rvo2_ros2__msg__ElementCharacteristicsStamped(
  eprosima::fastcdr::Cdr &,
  rvo2_ros2__msg__ElementCharacteristicsStamped * ros_message);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_rvo2_ros2
size_t get_serialized_size_rvo2_ros2__msg__ElementCharacteristicsStamped(
  const void * untyped_ros_message,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_rvo2_ros2
size_t max_serialized_size_rvo2_ros2__msg__ElementCharacteristicsStamped(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_rvo2_ros2
bool cdr_serialize_key_rvo2_ros2__msg__ElementCharacteristicsStamped(
  const rvo2_ros2__msg__ElementCharacteristicsStamped * ros_message,
  eprosima::fastcdr::Cdr & cdr);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_rvo2_ros2
size_t get_serialized_size_key_rvo2_ros2__msg__ElementCharacteristicsStamped(
  const void * untyped_ros_message,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_rvo2_ros2
size_t max_serialized_size_key_rvo2_ros2__msg__ElementCharacteristicsStamped(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_rvo2_ros2
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, rvo2_ros2, msg, ElementCharacteristicsStamped)();

#ifdef __cplusplus
}
#endif

#endif  // RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__ROSIDL_TYPESUPPORT_FASTRTPS_C_H_
