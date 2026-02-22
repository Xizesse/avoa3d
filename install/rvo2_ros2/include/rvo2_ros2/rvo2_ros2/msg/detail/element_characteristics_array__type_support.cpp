// generated from rosidl_typesupport_introspection_cpp/resource/idl__type_support.cpp.em
// with input from rvo2_ros2:msg/ElementCharacteristicsArray.idl
// generated code does not contain a copyright notice

#include "array"
#include "cstddef"
#include "string"
#include "vector"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_interface/macros.h"
#include "rvo2_ros2/msg/detail/element_characteristics_array__functions.h"
#include "rvo2_ros2/msg/detail/element_characteristics_array__struct.hpp"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/message_type_support_decl.hpp"
#include "rosidl_typesupport_introspection_cpp/visibility_control.h"

namespace rvo2_ros2
{

namespace msg
{

namespace rosidl_typesupport_introspection_cpp
{

void ElementCharacteristicsArray_init_function(
  void * message_memory, rosidl_runtime_cpp::MessageInitialization _init)
{
  new (message_memory) rvo2_ros2::msg::ElementCharacteristicsArray(_init);
}

void ElementCharacteristicsArray_fini_function(void * message_memory)
{
  auto typed_message = static_cast<rvo2_ros2::msg::ElementCharacteristicsArray *>(message_memory);
  typed_message->~ElementCharacteristicsArray();
}

size_t size_function__ElementCharacteristicsArray__elements(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<rvo2_ros2::msg::ElementCharacteristicsStamped> *>(untyped_member);
  return member->size();
}

const void * get_const_function__ElementCharacteristicsArray__elements(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<rvo2_ros2::msg::ElementCharacteristicsStamped> *>(untyped_member);
  return &member[index];
}

void * get_function__ElementCharacteristicsArray__elements(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<rvo2_ros2::msg::ElementCharacteristicsStamped> *>(untyped_member);
  return &member[index];
}

void fetch_function__ElementCharacteristicsArray__elements(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const rvo2_ros2::msg::ElementCharacteristicsStamped *>(
    get_const_function__ElementCharacteristicsArray__elements(untyped_member, index));
  auto & value = *reinterpret_cast<rvo2_ros2::msg::ElementCharacteristicsStamped *>(untyped_value);
  value = item;
}

void assign_function__ElementCharacteristicsArray__elements(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<rvo2_ros2::msg::ElementCharacteristicsStamped *>(
    get_function__ElementCharacteristicsArray__elements(untyped_member, index));
  const auto & value = *reinterpret_cast<const rvo2_ros2::msg::ElementCharacteristicsStamped *>(untyped_value);
  item = value;
}

void resize_function__ElementCharacteristicsArray__elements(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<rvo2_ros2::msg::ElementCharacteristicsStamped> *>(untyped_member);
  member->resize(size);
}

static const ::rosidl_typesupport_introspection_cpp::MessageMember ElementCharacteristicsArray_message_member_array[1] = {
  {
    "elements",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    ::rosidl_typesupport_introspection_cpp::get_message_type_support_handle<rvo2_ros2::msg::ElementCharacteristicsStamped>(),  // members of sub message
    false,  // is key
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(rvo2_ros2::msg::ElementCharacteristicsArray, elements),  // bytes offset in struct
    nullptr,  // default value
    size_function__ElementCharacteristicsArray__elements,  // size() function pointer
    get_const_function__ElementCharacteristicsArray__elements,  // get_const(index) function pointer
    get_function__ElementCharacteristicsArray__elements,  // get(index) function pointer
    fetch_function__ElementCharacteristicsArray__elements,  // fetch(index, &value) function pointer
    assign_function__ElementCharacteristicsArray__elements,  // assign(index, value) function pointer
    resize_function__ElementCharacteristicsArray__elements  // resize(index) function pointer
  }
};

static const ::rosidl_typesupport_introspection_cpp::MessageMembers ElementCharacteristicsArray_message_members = {
  "rvo2_ros2::msg",  // message namespace
  "ElementCharacteristicsArray",  // message name
  1,  // number of fields
  sizeof(rvo2_ros2::msg::ElementCharacteristicsArray),
  false,  // has_any_key_member_
  ElementCharacteristicsArray_message_member_array,  // message members
  ElementCharacteristicsArray_init_function,  // function to initialize message memory (memory has to be allocated)
  ElementCharacteristicsArray_fini_function  // function to terminate message instance (will not free memory)
};

static const rosidl_message_type_support_t ElementCharacteristicsArray_message_type_support_handle = {
  ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  &ElementCharacteristicsArray_message_members,
  get_message_typesupport_handle_function,
  &rvo2_ros2__msg__ElementCharacteristicsArray__get_type_hash,
  &rvo2_ros2__msg__ElementCharacteristicsArray__get_type_description,
  &rvo2_ros2__msg__ElementCharacteristicsArray__get_type_description_sources,
};

}  // namespace rosidl_typesupport_introspection_cpp

}  // namespace msg

}  // namespace rvo2_ros2


namespace rosidl_typesupport_introspection_cpp
{

template<>
ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<rvo2_ros2::msg::ElementCharacteristicsArray>()
{
  return &::rvo2_ros2::msg::rosidl_typesupport_introspection_cpp::ElementCharacteristicsArray_message_type_support_handle;
}

}  // namespace rosidl_typesupport_introspection_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, rvo2_ros2, msg, ElementCharacteristicsArray)() {
  return &::rvo2_ros2::msg::rosidl_typesupport_introspection_cpp::ElementCharacteristicsArray_message_type_support_handle;
}

#ifdef __cplusplus
}
#endif
