// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from avoa3d:msg/ElementCharacteristicsArray.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "avoa3d/msg/detail/element_characteristics_array__rosidl_typesupport_introspection_c.h"
#include "avoa3d/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "avoa3d/msg/detail/element_characteristics_array__functions.h"
#include "avoa3d/msg/detail/element_characteristics_array__struct.h"


// Include directives for member types
// Member `elements`
#include "avoa3d/msg/element_characteristics_stamped.h"
// Member `elements`
#include "avoa3d/msg/detail/element_characteristics_stamped__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  avoa3d__msg__ElementCharacteristicsArray__init(message_memory);
}

void avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_fini_function(void * message_memory)
{
  avoa3d__msg__ElementCharacteristicsArray__fini(message_memory);
}

size_t avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__size_function__ElementCharacteristicsArray__elements(
  const void * untyped_member)
{
  const avoa3d__msg__ElementCharacteristicsStamped__Sequence * member =
    (const avoa3d__msg__ElementCharacteristicsStamped__Sequence *)(untyped_member);
  return member->size;
}

const void * avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__get_const_function__ElementCharacteristicsArray__elements(
  const void * untyped_member, size_t index)
{
  const avoa3d__msg__ElementCharacteristicsStamped__Sequence * member =
    (const avoa3d__msg__ElementCharacteristicsStamped__Sequence *)(untyped_member);
  return &member->data[index];
}

void * avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__get_function__ElementCharacteristicsArray__elements(
  void * untyped_member, size_t index)
{
  avoa3d__msg__ElementCharacteristicsStamped__Sequence * member =
    (avoa3d__msg__ElementCharacteristicsStamped__Sequence *)(untyped_member);
  return &member->data[index];
}

void avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__fetch_function__ElementCharacteristicsArray__elements(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const avoa3d__msg__ElementCharacteristicsStamped * item =
    ((const avoa3d__msg__ElementCharacteristicsStamped *)
    avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__get_const_function__ElementCharacteristicsArray__elements(untyped_member, index));
  avoa3d__msg__ElementCharacteristicsStamped * value =
    (avoa3d__msg__ElementCharacteristicsStamped *)(untyped_value);
  *value = *item;
}

void avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__assign_function__ElementCharacteristicsArray__elements(
  void * untyped_member, size_t index, const void * untyped_value)
{
  avoa3d__msg__ElementCharacteristicsStamped * item =
    ((avoa3d__msg__ElementCharacteristicsStamped *)
    avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__get_function__ElementCharacteristicsArray__elements(untyped_member, index));
  const avoa3d__msg__ElementCharacteristicsStamped * value =
    (const avoa3d__msg__ElementCharacteristicsStamped *)(untyped_value);
  *item = *value;
}

bool avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__resize_function__ElementCharacteristicsArray__elements(
  void * untyped_member, size_t size)
{
  avoa3d__msg__ElementCharacteristicsStamped__Sequence * member =
    (avoa3d__msg__ElementCharacteristicsStamped__Sequence *)(untyped_member);
  avoa3d__msg__ElementCharacteristicsStamped__Sequence__fini(member);
  return avoa3d__msg__ElementCharacteristicsStamped__Sequence__init(member, size);
}

static rosidl_typesupport_introspection_c__MessageMember avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_member_array[1] = {
  {
    "elements",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is key
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(avoa3d__msg__ElementCharacteristicsArray, elements),  // bytes offset in struct
    NULL,  // default value
    avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__size_function__ElementCharacteristicsArray__elements,  // size() function pointer
    avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__get_const_function__ElementCharacteristicsArray__elements,  // get_const(index) function pointer
    avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__get_function__ElementCharacteristicsArray__elements,  // get(index) function pointer
    avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__fetch_function__ElementCharacteristicsArray__elements,  // fetch(index, &value) function pointer
    avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__assign_function__ElementCharacteristicsArray__elements,  // assign(index, value) function pointer
    avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__resize_function__ElementCharacteristicsArray__elements  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_members = {
  "avoa3d__msg",  // message namespace
  "ElementCharacteristicsArray",  // message name
  1,  // number of fields
  sizeof(avoa3d__msg__ElementCharacteristicsArray),
  false,  // has_any_key_member_
  avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_member_array,  // message members
  avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_init_function,  // function to initialize message memory (memory has to be allocated)
  avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_type_support_handle = {
  0,
  &avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_members,
  get_message_typesupport_handle_function,
  &avoa3d__msg__ElementCharacteristicsArray__get_type_hash,
  &avoa3d__msg__ElementCharacteristicsArray__get_type_description,
  &avoa3d__msg__ElementCharacteristicsArray__get_type_description_sources,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_avoa3d
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, avoa3d, msg, ElementCharacteristicsArray)() {
  avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_member_array[0].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, avoa3d, msg, ElementCharacteristicsStamped)();
  if (!avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_type_support_handle.typesupport_identifier) {
    avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &avoa3d__msg__ElementCharacteristicsArray__rosidl_typesupport_introspection_c__ElementCharacteristicsArray_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
