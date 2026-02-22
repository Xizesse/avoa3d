// generated from rosidl_typesupport_fastrtps_c/resource/idl__rosidl_typesupport_fastrtps_c.h.em
// with input from avoa3d:msg/ElementCharacteristicsArray.idl
// generated code does not contain a copyright notice
#ifndef AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__ROSIDL_TYPESUPPORT_FASTRTPS_C_H_
#define AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__ROSIDL_TYPESUPPORT_FASTRTPS_C_H_


#include <stddef.h>
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_interface/macros.h"
#include "avoa3d/msg/rosidl_typesupport_fastrtps_c__visibility_control.h"
#include "avoa3d/msg/detail/element_characteristics_array__struct.h"
#include "fastcdr/Cdr.h"

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_avoa3d
bool cdr_serialize_avoa3d__msg__ElementCharacteristicsArray(
  const avoa3d__msg__ElementCharacteristicsArray * ros_message,
  eprosima::fastcdr::Cdr & cdr);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_avoa3d
bool cdr_deserialize_avoa3d__msg__ElementCharacteristicsArray(
  eprosima::fastcdr::Cdr &,
  avoa3d__msg__ElementCharacteristicsArray * ros_message);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_avoa3d
size_t get_serialized_size_avoa3d__msg__ElementCharacteristicsArray(
  const void * untyped_ros_message,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_avoa3d
size_t max_serialized_size_avoa3d__msg__ElementCharacteristicsArray(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_avoa3d
bool cdr_serialize_key_avoa3d__msg__ElementCharacteristicsArray(
  const avoa3d__msg__ElementCharacteristicsArray * ros_message,
  eprosima::fastcdr::Cdr & cdr);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_avoa3d
size_t get_serialized_size_key_avoa3d__msg__ElementCharacteristicsArray(
  const void * untyped_ros_message,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_avoa3d
size_t max_serialized_size_key_avoa3d__msg__ElementCharacteristicsArray(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_avoa3d
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, avoa3d, msg, ElementCharacteristicsArray)();

#ifdef __cplusplus
}
#endif

#endif  // AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__ROSIDL_TYPESUPPORT_FASTRTPS_C_H_
