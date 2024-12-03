// generated from rosidl_typesupport_fastrtps_cpp/resource/idl__rosidl_typesupport_fastrtps_cpp.hpp.em
// with input from avoa:msg/ElementCharacteristicsStamped.idl
// generated code does not contain a copyright notice

#ifndef AVOA__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__ROSIDL_TYPESUPPORT_FASTRTPS_CPP_HPP_
#define AVOA__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__ROSIDL_TYPESUPPORT_FASTRTPS_CPP_HPP_

#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_interface/macros.h"
#include "avoa/msg/rosidl_typesupport_fastrtps_cpp__visibility_control.h"
#include "avoa/msg/detail/element_characteristics_stamped__struct.hpp"

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

namespace avoa
{

namespace msg
{

namespace typesupport_fastrtps_cpp
{

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_avoa
cdr_serialize(
  const avoa::msg::ElementCharacteristicsStamped & ros_message,
  eprosima::fastcdr::Cdr & cdr);

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_avoa
cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  avoa::msg::ElementCharacteristicsStamped & ros_message);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_avoa
get_serialized_size(
  const avoa::msg::ElementCharacteristicsStamped & ros_message,
  size_t current_alignment);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_avoa
max_serialized_size_ElementCharacteristicsStamped(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

}  // namespace typesupport_fastrtps_cpp

}  // namespace msg

}  // namespace avoa

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_avoa
const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, avoa, msg, ElementCharacteristicsStamped)();

#ifdef __cplusplus
}
#endif

#endif  // AVOA__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__ROSIDL_TYPESUPPORT_FASTRTPS_CPP_HPP_
