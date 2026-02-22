// generated from rosidl_typesupport_fastrtps_cpp/resource/idl__rosidl_typesupport_fastrtps_cpp.hpp.em
// with input from avoa3d:msg/ElementCharacteristicsStamped.idl
// generated code does not contain a copyright notice

#ifndef AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__ROSIDL_TYPESUPPORT_FASTRTPS_CPP_HPP_
#define AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__ROSIDL_TYPESUPPORT_FASTRTPS_CPP_HPP_

#include <cstddef>
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_interface/macros.h"
#include "avoa3d/msg/rosidl_typesupport_fastrtps_cpp__visibility_control.h"
#include "avoa3d/msg/detail/element_characteristics_stamped__struct.hpp"

#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-parameter"
# ifdef __clang__
#  pragma clang diagnostic ignored "-Wdeprecated-register"
#  pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
# endif
#endif
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

#include "fastcdr/Cdr.h"

namespace avoa3d
{

namespace msg
{

namespace typesupport_fastrtps_cpp
{

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_avoa3d
cdr_serialize(
  const avoa3d::msg::ElementCharacteristicsStamped & ros_message,
  eprosima::fastcdr::Cdr & cdr);

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_avoa3d
cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  avoa3d::msg::ElementCharacteristicsStamped & ros_message);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_avoa3d
get_serialized_size(
  const avoa3d::msg::ElementCharacteristicsStamped & ros_message,
  size_t current_alignment);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_avoa3d
max_serialized_size_ElementCharacteristicsStamped(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_avoa3d
cdr_serialize_key(
  const avoa3d::msg::ElementCharacteristicsStamped & ros_message,
  eprosima::fastcdr::Cdr &);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_avoa3d
get_serialized_size_key(
  const avoa3d::msg::ElementCharacteristicsStamped & ros_message,
  size_t current_alignment);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_avoa3d
max_serialized_size_key_ElementCharacteristicsStamped(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

}  // namespace typesupport_fastrtps_cpp

}  // namespace msg

}  // namespace avoa3d

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_avoa3d
const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, avoa3d, msg, ElementCharacteristicsStamped)();

#ifdef __cplusplus
}
#endif

#endif  // AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__ROSIDL_TYPESUPPORT_FASTRTPS_CPP_HPP_
