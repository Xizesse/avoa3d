// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from rvo2_ros2:msg/ElementCharacteristicsArray.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "rvo2_ros2/msg/element_characteristics_array.h"


#ifndef RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__STRUCT_H_
#define RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Constants defined in the message

// Include directives for member types
// Member 'elements'
#include "rvo2_ros2/msg/detail/element_characteristics_stamped__struct.h"

/// Struct defined in msg/ElementCharacteristicsArray in the package rvo2_ros2.
typedef struct rvo2_ros2__msg__ElementCharacteristicsArray
{
  rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence elements;
} rvo2_ros2__msg__ElementCharacteristicsArray;

// Struct for a sequence of rvo2_ros2__msg__ElementCharacteristicsArray.
typedef struct rvo2_ros2__msg__ElementCharacteristicsArray__Sequence
{
  rvo2_ros2__msg__ElementCharacteristicsArray * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} rvo2_ros2__msg__ElementCharacteristicsArray__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__STRUCT_H_
