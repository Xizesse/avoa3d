// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from avoa3d:msg/ElementCharacteristicsStamped.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "avoa3d/msg/element_characteristics_stamped.h"


#ifndef AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__STRUCT_H_
#define AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Constants defined in the message

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.h"
// Member 'pose'
#include "geometry_msgs/msg/detail/pose__struct.h"
// Member 'velocity'
// Member 'size'
#include "geometry_msgs/msg/detail/vector3__struct.h"

/// Struct defined in msg/ElementCharacteristicsStamped in the package avoa3d.
/**
  * Timestamp, frame_id definition - Relative to the working frame
 */
typedef struct avoa3d__msg__ElementCharacteristicsStamped
{
  std_msgs__msg__Header header;
  /// Number of elements in the scene - 0...N
  int32_t id;
  /// Defines the class of each element - 0...M
  int32_t type;
  /// Define if the object is static (False) or dynamic (True)
  bool dynamic;
  /// Current pose of the observed object
  geometry_msgs__msg__Pose pose;
  /// Position uncertainty (standard deviation for x, y, z)
  float position_x_std;
  float position_y_std;
  float position_z_std;
  /// Velocity (Vx, Vy, Vz) in the observer base_link
  geometry_msgs__msg__Vector3 velocity;
  /// Velocity uncertainty (standard deviation for vx, vy, vz)
  float velocity_x_std;
  float velocity_y_std;
  float velocity_z_std;
  /// Geometric dimensions of the element observed
  geometry_msgs__msg__Vector3 size;
  /// Radius uncertainty (standard deviation)
  float radius_std;
  /// Avoid zone around obstacles
  float protective_zone;
} avoa3d__msg__ElementCharacteristicsStamped;

// Struct for a sequence of avoa3d__msg__ElementCharacteristicsStamped.
typedef struct avoa3d__msg__ElementCharacteristicsStamped__Sequence
{
  avoa3d__msg__ElementCharacteristicsStamped * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} avoa3d__msg__ElementCharacteristicsStamped__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__STRUCT_H_
