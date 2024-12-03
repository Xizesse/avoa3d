// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from avoa:msg/ElementCharacteristicsStamped.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "avoa/msg/detail/element_characteristics_stamped__rosidl_typesupport_introspection_c.h"
#include "avoa/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "avoa/msg/detail/element_characteristics_stamped__functions.h"
#include "avoa/msg/detail/element_characteristics_stamped__struct.h"


// Include directives for member types
// Member `header`
#include "std_msgs/msg/header.h"
// Member `header`
#include "std_msgs/msg/detail/header__rosidl_typesupport_introspection_c.h"
// Member `pose`
#include "geometry_msgs/msg/pose.h"
// Member `pose`
#include "geometry_msgs/msg/detail/pose__rosidl_typesupport_introspection_c.h"
// Member `velocity`
// Member `size`
#include "geometry_msgs/msg/vector3.h"
// Member `velocity`
// Member `size`
#include "geometry_msgs/msg/detail/vector3__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void avoa__msg__ElementCharacteristicsStamped__rosidl_typesupport_introspection_c__ElementCharacteristicsStamped_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  avoa__msg__ElementCharacteristicsStamped__init(message_memory);
}

void avoa__msg__ElementCharacteristicsStamped__rosidl_typesupport_introspection_c__ElementCharacteristicsStamped_fini_function(void * message_memory)
{
  avoa__msg__ElementCharacteristicsStamped__fini(message_memory);
}

static rosidl_typesupport_introspection_c__MessageMember avoa__msg__ElementCharacteristicsStamped__rosidl_typesupport_introspection_c__ElementCharacteristicsStamped_message_member_array[8] = {
  {
    "header",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(avoa__msg__ElementCharacteristicsStamped, header),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "id",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_INT32,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(avoa__msg__ElementCharacteristicsStamped, id),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "type",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_INT32,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(avoa__msg__ElementCharacteristicsStamped, type),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "dynamic",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_BOOLEAN,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(avoa__msg__ElementCharacteristicsStamped, dynamic),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "pose",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(avoa__msg__ElementCharacteristicsStamped, pose),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "velocity",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(avoa__msg__ElementCharacteristicsStamped, velocity),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "size",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(avoa__msg__ElementCharacteristicsStamped, size),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "protective_zone",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(avoa__msg__ElementCharacteristicsStamped, protective_zone),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers avoa__msg__ElementCharacteristicsStamped__rosidl_typesupport_introspection_c__ElementCharacteristicsStamped_message_members = {
  "avoa__msg",  // message namespace
  "ElementCharacteristicsStamped",  // message name
  8,  // number of fields
  sizeof(avoa__msg__ElementCharacteristicsStamped),
  avoa__msg__ElementCharacteristicsStamped__rosidl_typesupport_introspection_c__ElementCharacteristicsStamped_message_member_array,  // message members
  avoa__msg__ElementCharacteristicsStamped__rosidl_typesupport_introspection_c__ElementCharacteristicsStamped_init_function,  // function to initialize message memory (memory has to be allocated)
  avoa__msg__ElementCharacteristicsStamped__rosidl_typesupport_introspection_c__ElementCharacteristicsStamped_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t avoa__msg__ElementCharacteristicsStamped__rosidl_typesupport_introspection_c__ElementCharacteristicsStamped_message_type_support_handle = {
  0,
  &avoa__msg__ElementCharacteristicsStamped__rosidl_typesupport_introspection_c__ElementCharacteristicsStamped_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_avoa
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, avoa, msg, ElementCharacteristicsStamped)() {
  avoa__msg__ElementCharacteristicsStamped__rosidl_typesupport_introspection_c__ElementCharacteristicsStamped_message_member_array[0].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, std_msgs, msg, Header)();
  avoa__msg__ElementCharacteristicsStamped__rosidl_typesupport_introspection_c__ElementCharacteristicsStamped_message_member_array[4].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, geometry_msgs, msg, Pose)();
  avoa__msg__ElementCharacteristicsStamped__rosidl_typesupport_introspection_c__ElementCharacteristicsStamped_message_member_array[5].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, geometry_msgs, msg, Vector3)();
  avoa__msg__ElementCharacteristicsStamped__rosidl_typesupport_introspection_c__ElementCharacteristicsStamped_message_member_array[6].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, geometry_msgs, msg, Vector3)();
  if (!avoa__msg__ElementCharacteristicsStamped__rosidl_typesupport_introspection_c__ElementCharacteristicsStamped_message_type_support_handle.typesupport_identifier) {
    avoa__msg__ElementCharacteristicsStamped__rosidl_typesupport_introspection_c__ElementCharacteristicsStamped_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &avoa__msg__ElementCharacteristicsStamped__rosidl_typesupport_introspection_c__ElementCharacteristicsStamped_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
