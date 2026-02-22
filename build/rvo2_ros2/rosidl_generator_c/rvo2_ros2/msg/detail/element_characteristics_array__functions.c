// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from rvo2_ros2:msg/ElementCharacteristicsArray.idl
// generated code does not contain a copyright notice
#include "rvo2_ros2/msg/detail/element_characteristics_array__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `elements`
#include "rvo2_ros2/msg/detail/element_characteristics_stamped__functions.h"

bool
rvo2_ros2__msg__ElementCharacteristicsArray__init(rvo2_ros2__msg__ElementCharacteristicsArray * msg)
{
  if (!msg) {
    return false;
  }
  // elements
  if (!rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence__init(&msg->elements, 0)) {
    rvo2_ros2__msg__ElementCharacteristicsArray__fini(msg);
    return false;
  }
  return true;
}

void
rvo2_ros2__msg__ElementCharacteristicsArray__fini(rvo2_ros2__msg__ElementCharacteristicsArray * msg)
{
  if (!msg) {
    return;
  }
  // elements
  rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence__fini(&msg->elements);
}

bool
rvo2_ros2__msg__ElementCharacteristicsArray__are_equal(const rvo2_ros2__msg__ElementCharacteristicsArray * lhs, const rvo2_ros2__msg__ElementCharacteristicsArray * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // elements
  if (!rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence__are_equal(
      &(lhs->elements), &(rhs->elements)))
  {
    return false;
  }
  return true;
}

bool
rvo2_ros2__msg__ElementCharacteristicsArray__copy(
  const rvo2_ros2__msg__ElementCharacteristicsArray * input,
  rvo2_ros2__msg__ElementCharacteristicsArray * output)
{
  if (!input || !output) {
    return false;
  }
  // elements
  if (!rvo2_ros2__msg__ElementCharacteristicsStamped__Sequence__copy(
      &(input->elements), &(output->elements)))
  {
    return false;
  }
  return true;
}

rvo2_ros2__msg__ElementCharacteristicsArray *
rvo2_ros2__msg__ElementCharacteristicsArray__create(void)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rvo2_ros2__msg__ElementCharacteristicsArray * msg = (rvo2_ros2__msg__ElementCharacteristicsArray *)allocator.allocate(sizeof(rvo2_ros2__msg__ElementCharacteristicsArray), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(rvo2_ros2__msg__ElementCharacteristicsArray));
  bool success = rvo2_ros2__msg__ElementCharacteristicsArray__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
rvo2_ros2__msg__ElementCharacteristicsArray__destroy(rvo2_ros2__msg__ElementCharacteristicsArray * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    rvo2_ros2__msg__ElementCharacteristicsArray__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
rvo2_ros2__msg__ElementCharacteristicsArray__Sequence__init(rvo2_ros2__msg__ElementCharacteristicsArray__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rvo2_ros2__msg__ElementCharacteristicsArray * data = NULL;

  if (size) {
    data = (rvo2_ros2__msg__ElementCharacteristicsArray *)allocator.zero_allocate(size, sizeof(rvo2_ros2__msg__ElementCharacteristicsArray), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = rvo2_ros2__msg__ElementCharacteristicsArray__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        rvo2_ros2__msg__ElementCharacteristicsArray__fini(&data[i - 1]);
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
rvo2_ros2__msg__ElementCharacteristicsArray__Sequence__fini(rvo2_ros2__msg__ElementCharacteristicsArray__Sequence * array)
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
      rvo2_ros2__msg__ElementCharacteristicsArray__fini(&array->data[i]);
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

rvo2_ros2__msg__ElementCharacteristicsArray__Sequence *
rvo2_ros2__msg__ElementCharacteristicsArray__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rvo2_ros2__msg__ElementCharacteristicsArray__Sequence * array = (rvo2_ros2__msg__ElementCharacteristicsArray__Sequence *)allocator.allocate(sizeof(rvo2_ros2__msg__ElementCharacteristicsArray__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = rvo2_ros2__msg__ElementCharacteristicsArray__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
rvo2_ros2__msg__ElementCharacteristicsArray__Sequence__destroy(rvo2_ros2__msg__ElementCharacteristicsArray__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    rvo2_ros2__msg__ElementCharacteristicsArray__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
rvo2_ros2__msg__ElementCharacteristicsArray__Sequence__are_equal(const rvo2_ros2__msg__ElementCharacteristicsArray__Sequence * lhs, const rvo2_ros2__msg__ElementCharacteristicsArray__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!rvo2_ros2__msg__ElementCharacteristicsArray__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
rvo2_ros2__msg__ElementCharacteristicsArray__Sequence__copy(
  const rvo2_ros2__msg__ElementCharacteristicsArray__Sequence * input,
  rvo2_ros2__msg__ElementCharacteristicsArray__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(rvo2_ros2__msg__ElementCharacteristicsArray);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    rvo2_ros2__msg__ElementCharacteristicsArray * data =
      (rvo2_ros2__msg__ElementCharacteristicsArray *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!rvo2_ros2__msg__ElementCharacteristicsArray__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          rvo2_ros2__msg__ElementCharacteristicsArray__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!rvo2_ros2__msg__ElementCharacteristicsArray__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
