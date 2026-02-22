// generated from rosidl_typesupport_fastrtps_c/resource/idl__type_support_c.cpp.em
// with input from avoa3d:msg/ElementCharacteristicsArray.idl
// generated code does not contain a copyright notice
#include "avoa3d/msg/detail/element_characteristics_array__rosidl_typesupport_fastrtps_c.h"


#include <cassert>
#include <cstddef>
#include <limits>
#include <string>
#include "rosidl_typesupport_fastrtps_c/identifier.h"
#include "rosidl_typesupport_fastrtps_c/serialization_helpers.hpp"
#include "rosidl_typesupport_fastrtps_c/wstring_conversion.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"
#include "avoa3d/msg/rosidl_typesupport_fastrtps_c__visibility_control.h"
#include "avoa3d/msg/detail/element_characteristics_array__struct.h"
#include "avoa3d/msg/detail/element_characteristics_array__functions.h"
#include "fastcdr/Cdr.h"

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

// includes and forward declarations of message dependencies and their conversion functions

#if defined(__cplusplus)
extern "C"
{
#endif

#include "avoa3d/msg/detail/element_characteristics_stamped__functions.h"  // elements

// forward declare type support functions

bool cdr_serialize_avoa3d__msg__ElementCharacteristicsStamped(
  const avoa3d__msg__ElementCharacteristicsStamped * ros_message,
  eprosima::fastcdr::Cdr & cdr);

bool cdr_deserialize_avoa3d__msg__ElementCharacteristicsStamped(
  eprosima::fastcdr::Cdr & cdr,
  avoa3d__msg__ElementCharacteristicsStamped * ros_message);

size_t get_serialized_size_avoa3d__msg__ElementCharacteristicsStamped(
  const void * untyped_ros_message,
  size_t current_alignment);

size_t max_serialized_size_avoa3d__msg__ElementCharacteristicsStamped(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

bool cdr_serialize_key_avoa3d__msg__ElementCharacteristicsStamped(
  const avoa3d__msg__ElementCharacteristicsStamped * ros_message,
  eprosima::fastcdr::Cdr & cdr);

size_t get_serialized_size_key_avoa3d__msg__ElementCharacteristicsStamped(
  const void * untyped_ros_message,
  size_t current_alignment);

size_t max_serialized_size_key_avoa3d__msg__ElementCharacteristicsStamped(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, avoa3d, msg, ElementCharacteristicsStamped)();


using _ElementCharacteristicsArray__ros_msg_type = avoa3d__msg__ElementCharacteristicsArray;


ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_avoa3d
bool cdr_serialize_avoa3d__msg__ElementCharacteristicsArray(
  const avoa3d__msg__ElementCharacteristicsArray * ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  // Field name: elements
  {
    size_t size = ros_message->elements.size;
    auto array_ptr = ros_message->elements.data;
    cdr << static_cast<uint32_t>(size);
    for (size_t i = 0; i < size; ++i) {
      cdr_serialize_avoa3d__msg__ElementCharacteristicsStamped(
        &array_ptr[i], cdr);
    }
  }

  return true;
}

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_avoa3d
bool cdr_deserialize_avoa3d__msg__ElementCharacteristicsArray(
  eprosima::fastcdr::Cdr & cdr,
  avoa3d__msg__ElementCharacteristicsArray * ros_message)
{
  // Field name: elements
  {
    uint32_t cdrSize;
    cdr >> cdrSize;
    size_t size = static_cast<size_t>(cdrSize);

    // Check there are at least 'size' remaining bytes in the CDR stream before resizing
    auto old_state = cdr.get_state();
    bool correct_size = cdr.jump(size);
    cdr.set_state(old_state);
    if (!correct_size) {
      fprintf(stderr, "sequence size exceeds remaining buffer\n");
      return false;
    }

    if (ros_message->elements.data) {
      avoa3d__msg__ElementCharacteristicsStamped__Sequence__fini(&ros_message->elements);
    }
    if (!avoa3d__msg__ElementCharacteristicsStamped__Sequence__init(&ros_message->elements, size)) {
      fprintf(stderr, "failed to create array for field 'elements'");
      return false;
    }
    auto array_ptr = ros_message->elements.data;
    for (size_t i = 0; i < size; ++i) {
      cdr_deserialize_avoa3d__msg__ElementCharacteristicsStamped(cdr, &array_ptr[i]);
    }
  }

  return true;
}  // NOLINT(readability/fn_size)


ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_avoa3d
size_t get_serialized_size_avoa3d__msg__ElementCharacteristicsArray(
  const void * untyped_ros_message,
  size_t current_alignment)
{
  const _ElementCharacteristicsArray__ros_msg_type * ros_message = static_cast<const _ElementCharacteristicsArray__ros_msg_type *>(untyped_ros_message);
  (void)ros_message;
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  (void)padding;
  (void)wchar_size;

  // Field name: elements
  {
    size_t array_size = ros_message->elements.size;
    auto array_ptr = ros_message->elements.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    for (size_t index = 0; index < array_size; ++index) {
      current_alignment += get_serialized_size_avoa3d__msg__ElementCharacteristicsStamped(
        &array_ptr[index], current_alignment);
    }
  }

  return current_alignment - initial_alignment;
}


ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_avoa3d
size_t max_serialized_size_avoa3d__msg__ElementCharacteristicsArray(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment)
{
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  size_t last_member_size = 0;
  (void)last_member_size;
  (void)padding;
  (void)wchar_size;

  full_bounded = true;
  is_plain = true;

  // Field name: elements
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size;
      inner_size =
        max_serialized_size_avoa3d__msg__ElementCharacteristicsStamped(
        inner_full_bounded, inner_is_plain, current_alignment);
      last_member_size += inner_size;
      current_alignment += inner_size;
      full_bounded &= inner_full_bounded;
      is_plain &= inner_is_plain;
    }
  }


  size_t ret_val = current_alignment - initial_alignment;
  if (is_plain) {
    // All members are plain, and type is not empty.
    // We still need to check that the in-memory alignment
    // is the same as the CDR mandated alignment.
    using DataType = avoa3d__msg__ElementCharacteristicsArray;
    is_plain =
      (
      offsetof(DataType, elements) +
      last_member_size
      ) == ret_val;
  }
  return ret_val;
}

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_avoa3d
bool cdr_serialize_key_avoa3d__msg__ElementCharacteristicsArray(
  const avoa3d__msg__ElementCharacteristicsArray * ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  // Field name: elements
  {
    size_t size = ros_message->elements.size;
    auto array_ptr = ros_message->elements.data;
    cdr << static_cast<uint32_t>(size);
    for (size_t i = 0; i < size; ++i) {
      cdr_serialize_key_avoa3d__msg__ElementCharacteristicsStamped(
        &array_ptr[i], cdr);
    }
  }

  return true;
}

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_avoa3d
size_t get_serialized_size_key_avoa3d__msg__ElementCharacteristicsArray(
  const void * untyped_ros_message,
  size_t current_alignment)
{
  const _ElementCharacteristicsArray__ros_msg_type * ros_message = static_cast<const _ElementCharacteristicsArray__ros_msg_type *>(untyped_ros_message);
  (void)ros_message;

  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  (void)padding;
  (void)wchar_size;

  // Field name: elements
  {
    size_t array_size = ros_message->elements.size;
    auto array_ptr = ros_message->elements.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    for (size_t index = 0; index < array_size; ++index) {
      current_alignment += get_serialized_size_key_avoa3d__msg__ElementCharacteristicsStamped(
        &array_ptr[index], current_alignment);
    }
  }

  return current_alignment - initial_alignment;
}

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_avoa3d
size_t max_serialized_size_key_avoa3d__msg__ElementCharacteristicsArray(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment)
{
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  size_t last_member_size = 0;
  (void)last_member_size;
  (void)padding;
  (void)wchar_size;

  full_bounded = true;
  is_plain = true;
  // Field name: elements
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size;
      inner_size =
        max_serialized_size_key_avoa3d__msg__ElementCharacteristicsStamped(
        inner_full_bounded, inner_is_plain, current_alignment);
      last_member_size += inner_size;
      current_alignment += inner_size;
      full_bounded &= inner_full_bounded;
      is_plain &= inner_is_plain;
    }
  }

  size_t ret_val = current_alignment - initial_alignment;
  if (is_plain) {
    // All members are plain, and type is not empty.
    // We still need to check that the in-memory alignment
    // is the same as the CDR mandated alignment.
    using DataType = avoa3d__msg__ElementCharacteristicsArray;
    is_plain =
      (
      offsetof(DataType, elements) +
      last_member_size
      ) == ret_val;
  }
  return ret_val;
}


static bool _ElementCharacteristicsArray__cdr_serialize(
  const void * untyped_ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  const avoa3d__msg__ElementCharacteristicsArray * ros_message = static_cast<const avoa3d__msg__ElementCharacteristicsArray *>(untyped_ros_message);
  (void)ros_message;
  return cdr_serialize_avoa3d__msg__ElementCharacteristicsArray(ros_message, cdr);
}

static bool _ElementCharacteristicsArray__cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  void * untyped_ros_message)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  avoa3d__msg__ElementCharacteristicsArray * ros_message = static_cast<avoa3d__msg__ElementCharacteristicsArray *>(untyped_ros_message);
  (void)ros_message;
  return cdr_deserialize_avoa3d__msg__ElementCharacteristicsArray(cdr, ros_message);
}

static uint32_t _ElementCharacteristicsArray__get_serialized_size(const void * untyped_ros_message)
{
  return static_cast<uint32_t>(
    get_serialized_size_avoa3d__msg__ElementCharacteristicsArray(
      untyped_ros_message, 0));
}

static size_t _ElementCharacteristicsArray__max_serialized_size(char & bounds_info)
{
  bool full_bounded;
  bool is_plain;
  size_t ret_val;

  ret_val = max_serialized_size_avoa3d__msg__ElementCharacteristicsArray(
    full_bounded, is_plain, 0);

  bounds_info =
    is_plain ? ROSIDL_TYPESUPPORT_FASTRTPS_PLAIN_TYPE :
    full_bounded ? ROSIDL_TYPESUPPORT_FASTRTPS_BOUNDED_TYPE : ROSIDL_TYPESUPPORT_FASTRTPS_UNBOUNDED_TYPE;
  return ret_val;
}


static message_type_support_callbacks_t __callbacks_ElementCharacteristicsArray = {
  "avoa3d::msg",
  "ElementCharacteristicsArray",
  _ElementCharacteristicsArray__cdr_serialize,
  _ElementCharacteristicsArray__cdr_deserialize,
  _ElementCharacteristicsArray__get_serialized_size,
  _ElementCharacteristicsArray__max_serialized_size,
  nullptr
};

static rosidl_message_type_support_t _ElementCharacteristicsArray__type_support = {
  rosidl_typesupport_fastrtps_c__identifier,
  &__callbacks_ElementCharacteristicsArray,
  get_message_typesupport_handle_function,
  &avoa3d__msg__ElementCharacteristicsArray__get_type_hash,
  &avoa3d__msg__ElementCharacteristicsArray__get_type_description,
  &avoa3d__msg__ElementCharacteristicsArray__get_type_description_sources,
};

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, avoa3d, msg, ElementCharacteristicsArray)() {
  return &_ElementCharacteristicsArray__type_support;
}

#if defined(__cplusplus)
}
#endif
