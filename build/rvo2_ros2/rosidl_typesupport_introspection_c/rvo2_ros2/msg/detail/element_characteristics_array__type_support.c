// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from rvo2_ros2:msg/ElementCharacteristicsArray.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "rvo2_ros2/msg/detail/element_characteristics_array__rosidl_typesupport_introspection_c.h"
#include "rvo2_ros2/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "rvo2_ros2/msg/detail/element_characteristics_array__functions.h"
#include "rvo2_ros2/msg/detail/element_characteristics_array__struct.h"


// Include directives for member types
// Member `elements`
#include "rvo2_ros2/msg/element_characteristics_stamped.h"
// Member `elements`
#include "rvo2_ros2/msg/detail/element_characteristics_stamped__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  rvo2_ros2__msg__ElementCharacteristicsArray__init(message_memory);
}

void rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_fini_function(void * message_memory)
{
  rvo2_ros2__msg__ElementCharacteristicsArray__fini(message_memory);
}

size_t rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__size_function__ElementCharacteristicsArray__elements(
  const void * untyped_member)
{
  const rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence * member =
    (const rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence *)(untyped_member);
  return member->size;
}

const void * rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__get_const_function__ElementCharacteristicsArray__elements(
  const void * untyped_member, size_t index)
{
  const rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence * member =
    (const rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence *)(untyped_member);
  return &member->data[index];
}

void * rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__get_function__ElementCharacteristicsArray__elements(
  void * untyped_member, size_t index)
{
  rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence * member =
    (rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence *)(untyped_member);
  return &member->data[index];
}

void rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__fetch_function__ElementCharacteristicsArray__elements(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const rvo2_ros2__msg__ElementCharacteristicsStamped * item =
    ((const rvo2_ros2__msg__ElementCharacteristicsStamped *)
    rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__get_const_function__ElementCharacteristicsArray__elements(untyped_member, index));
  rvo2_ros2__msg__ElementCharacteristicsStamped * value =
    (rvo2_ros2__msg__ElementCharacteristicsStamped *)(untyped_value);
  *value = *item;
}

void rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__assign_function__ElementCharacteristicsArray__elements(
  void * untyped_member, size_t index, const void * untyped_value)
{
  rvo2_ros2__msg__ElementCharacteristicsStamped * item =
    ((rvo2_ros2__msg__ElementCharacteristicsStamped *)
    rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__get_function__ElementCharacteristicsArray__elements(untyped_member, index));
  const rvo2_ros2__msg__ElementCharacteristicsStamped * value =
    (const rvo2_ros2__msg__ElementCharacteristicsStamped *)(untyped_value);
  *item = *value;
}

bool rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__resize_function__ElementCharacteristicsArray__elements(
  void * untyped_member, size_t size)
{
  rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence * member =
    (rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence *)(untyped_member);
  rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence__fini(member);
  return rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence__init(member, size);
}

static rosidl_typesupport_introspection_c__MessageMember rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_member_array[1] = {
  {
    "elements",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is key
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(rvo2_ros2__msg__ElementCharacteristicsArray, elements),  // bytes offset in struct
    NULL,  // default value
    rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__size_function__ElementCharacteristicsArray__elements,  // size() function pointer
    rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__get_const_function__ElementCharacteristicsArray__elements,  // get_const(index) function pointer
    rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__get_function__ElementCharacteristicsArray__elements,  // get(index) function pointer
    rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__fetch_function__ElementCharacteristicsArray__elements,  // fetch(index, &value) function pointer
    rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__assign_function__ElementCharacteristicsArray__elements,  // assign(index, value) function pointer
    rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__resize_function__ElementCharacteristicsArray__elements  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_members = {
  "rvo2_ros2__msg",  // message namespace
  "ElementCharacteristicsArray",  // message name
  1,  // number of fields
  sizeof(rvo2_ros2__msg__ElementCharacteristicsArray),
  false,  // has_any_key_member_
  rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_member_array,  // message members
  rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_init_function,  // function to initialize message memory (memory has to be allocated)
  rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_type_support_handle = {
  0,
  &rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_members,
  get_message_typesupport_handle_function,
  &rvo2_ros2__msg__ElementCharacteristicsArray__get_type_hash,
  &rvo2_ros2__msg__ElementCharacteristicsArray__get_type_description,
  &rvo2_ros2__msg__ElementCharacteristicsArray__get_type_description_sources,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_rvo2_ros2
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, rvo2_ros2, msg, ElementCharacteristicsArray)() {
  rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_member_array[0].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, rvo2_ros2, msg, ElementCharacteristicsStamped)();
  if (!rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_type_support_handle.typesupport_identifier) {
    rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &rvo2_ros2__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
