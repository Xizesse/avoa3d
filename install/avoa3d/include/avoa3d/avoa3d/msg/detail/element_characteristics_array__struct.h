// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from avoa3d:msg/ElementCharacteristicsArray.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "avoa3d/msg/element_characteristics_array.h"


#ifndef AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__STRUCT_H_
#define AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__STRUCT_H_

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
#include "avoa3d/msg/detail/element_characteristics_stamped__struct.h"

/// Struct defined in msg/ElementCharacteristicsArray in the package avoa3d.
typedef struct avoa3d__msg__ElementCharacteristicsArray
{
  avoa3d__msg__ElementCharacteristicsStamped__Sequence elements;
} avoa3d__msg__ElementCharacteristicsArray;

// Struct for a sequence of avoa3d__msg__ElementCharacteristicsArray.
typedef struct avoa3d__msg__ElementCharacteristicsArray__Sequence
{
  avoa3d__msg__ElementCharacteristicsArray * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} avoa3d__msg__ElementCharacteristicsArray__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__STRUCT_H_
