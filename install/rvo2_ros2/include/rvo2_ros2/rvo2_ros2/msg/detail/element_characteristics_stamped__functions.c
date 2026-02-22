// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from rvo2_ros2:msg/ElementCharacteristicsStamped.idl
// generated code does not contain a copyright notice
#include "rvo2_ros2/msg/detail/element_characteristics_stamped__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `header`
#include "std_msgs/msg/detail/header__functions.h"
// Member `pose`
#include "geometry_msgs/msg/detail/pose__functions.h"
// Member `velocity`
// Member `size`
#include "geometry_msgs/msg/detail/vector3__functions.h"

bool
rvo2_ros2__msg__ElementCharacteristicsStamped__init(rvo2_ros2__msg__ElementCharacteristicsStamped * msg)
{
  if (!msg) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__init(&msg->header)) {
    rvo2_ros2__msg__ElementCharacteristicsStamped__fini(msg);
    return false;
  }
  // id
  // type
  // dynamic
  // pose
  if (!geometry_msgs__msg__Pose__init(&msg->pose)) {
    rvo2_ros2__msg__ElementCharacteristicsStamped__fini(msg);
    return false;
  }
  // position_x_std
  // position_y_std
  // position_z_std
  // velocity
  if (!geometry_msgs__msg__Vector3__init(&msg->velocity)) {
    rvo2_ros2__msg__ElementCharacteristicsStamped__fini(msg);
    return false;
  }
  // velocity_x_std
  // velocity_y_std
  // velocity_z_std
  // size
  if (!geometry_msgs__msg__Vector3__init(&msg->size)) {
    rvo2_ros2__msg__ElementCharacteristicsStamped__fini(msg);
    return false;
  }
  // radius_std
  // protective_zone
  return true;
}

void
rvo2_ros2__msg__ElementCharacteristicsStamped__fini(rvo2_ros2__msg__ElementCharacteristicsStamped * msg)
{
  if (!msg) {
    return;
  }
  // header
  std_msgs__msg__Header__fini(&msg->header);
  // id
  // type
  // dynamic
  // pose
  geometry_msgs__msg__Pose__fini(&msg->pose);
  // position_x_std
  // position_y_std
  // position_z_std
  // velocity
  geometry_msgs__msg__Vector3__fini(&msg->velocity);
  // velocity_x_std
  // velocity_y_std
  // velocity_z_std
  // size
  geometry_msgs__msg__Vector3__fini(&msg->size);
  // radius_std
  // protective_zone
}

bool
rvo2_ros2__msg__ElementCharacteristicsStamped__are_equal(const rvo2_ros2__msg__ElementCharacteristicsStamped * lhs, const rvo2_ros2__msg__ElementCharacteristicsStamped * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__are_equal(
      &(lhs->header), &(rhs->header)))
  {
    return false;
  }
  // id
  if (lhs->id != rhs->id) {
    return false;
  }
  // type
  if (lhs->type != rhs->type) {
    return false;
  }
  // dynamic
  if (lhs->dynamic != rhs->dynamic) {
    return false;
  }
  // pose
  if (!geometry_msgs__msg__Pose__are_equal(
      &(lhs->pose), &(rhs->pose)))
  {
    return false;
  }
  // position_x_std
  if (lhs->position_x_std != rhs->position_x_std) {
    return false;
  }
  // position_y_std
  if (lhs->position_y_std != rhs->position_y_std) {
    return false;
  }
  // position_z_std
  if (lhs->position_z_std != rhs->position_z_std) {
    return false;
  }
  // velocity
  if (!geometry_msgs__msg__Vector3__are_equal(
      &(lhs->velocity), &(rhs->velocity)))
  {
    return false;
  }
  // velocity_x_std
  if (lhs->velocity_x_std != rhs->velocity_x_std) {
    return false;
  }
  // velocity_y_std
  if (lhs->velocity_y_std != rhs->velocity_y_std) {
    return false;
  }
  // velocity_z_std
  if (lhs->velocity_z_std != rhs->velocity_z_std) {
    return false;
  }
  // size
  if (!geometry_msgs__msg__Vector3__are_equal(
      &(lhs->size), &(rhs->size)))
  {
    return false;
  }
  // radius_std
  if (lhs->radius_std != rhs->radius_std) {
    return false;
  }
  // protective_zone
  if (lhs->protective_zone != rhs->protective_zone) {
    return false;
  }
  return true;
}

bool
rvo2_ros2__msg__ElementCharacteristicsStamped__copy(
  const rvo2_ros2__msg__ElementCharacteristicsStamped * input,
  rvo2_ros2__msg__ElementCharacteristicsStamped * output)
{
  if (!input || !output) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__copy(
      &(input->header), &(output->header)))
  {
    return false;
  }
  // id
  output->id = input->id;
  // type
  output->type = input->type;
  // dynamic
  output->dynamic = input->dynamic;
  // pose
  if (!geometry_msgs__msg__Pose__copy(
      &(input->pose), &(output->pose)))
  {
    return false;
  }
  // position_x_std
  output->position_x_std = input->position_x_std;
  // position_y_std
  output->position_y_std = input->position_y_std;
  // position_z_std
  output->position_z_std = input->position_z_std;
  // velocity
  if (!geometry_msgs__msg__Vector3__copy(
      &(input->velocity), &(output->velocity)))
  {
    return false;
  }
  // velocity_x_std
  output->velocity_x_std = input->velocity_x_std;
  // velocity_y_std
  output->velocity_y_std = input->velocity_y_std;
  // velocity_z_std
  output->velocity_z_std = input->velocity_z_std;
  // size
  if (!geometry_msgs__msg__Vector3__copy(
      &(input->size), &(output->size)))
  {
    return false;
  }
  // radius_std
  output->radius_std = input->radius_std;
  // protective_zone
  output->protective_zone = input->protective_zone;
  return true;
}

rvo2_ros2__msg__ElementCharacteristicsStamped *
rvo2_ros2__msg__ElementCharacteristicsStamped__create(void)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rvo2_ros2__msg__ElementCharacteristicsStamped * msg = (rvo2_ros2__msg__ElementCharacteristicsStamped *)allocator.allocate(sizeof(rvo2_ros2__msg__ElementCharacteristicsStamped), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(rvo2_ros2__msg__ElementCharacteristicsStamped));
  bool success = rvo2_ros2__msg__ElementCharacteristicsStamped__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
rvo2_ros2__msg__ElementCharacteristicsStamped__destroy(rvo2_ros2__msg__ElementCharacteristicsStamped * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    rvo2_ros2__msg__ElementCharacteristicsStamped__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence__init(rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rvo2_ros2__msg__ElementCharacteristicsStamped * data = NULL;

  if (size) {
    data = (rvo2_ros2__msg__ElementCharacteristicsStamped *)allocator.zero_allocate(size, sizeof(rvo2_ros2__msg__ElementCharacteristicsStamped), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = rvo2_ros2__msg__ElementCharacteristicsStamped__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        rvo2_ros2__msg__ElementCharacteristicsStamped__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence__fini(rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      rvo2_ros2__msg__ElementCharacteristicsStamped__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence *
rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence * array = (rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence *)allocator.allocate(sizeof(rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence__destroy(rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence__are_equal(const rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence * lhs, const rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!rvo2_ros2__msg__ElementCharacteristicsStamped__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence__copy(
  const rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence * input,
  rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(rvo2_ros2__msg__ElementCharacteristicsStamped);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    rvo2_ros2__msg__ElementCharacteristicsStamped * data =
      (rvo2_ros2__msg__ElementCharacteristicsStamped *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!rvo2_ros2__msg__ElementCharacteristicsStamped__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          rvo2_ros2__msg__ElementCharacteristicsStamped__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!rvo2_ros2__msg__ElementCharacteristicsStamped__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
