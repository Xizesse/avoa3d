// generated from rosidl_generator_c/resource/idl__functions.h.em
// with input from avoa3d:msg/ElementCharacteristicsStamped.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "avoa3d/msg/element_characteristics_stamped.h"


#ifndef AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__FUNCTIONS_H_
#define AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__FUNCTIONS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "rosidl_runtime_c/action_type_support_struct.h"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_runtime_c/service_type_support_struct.h"
#include "rosidl_runtime_c/type_description/type_description__struct.h"
#include "rosidl_runtime_c/type_description/type_source__struct.h"
#include "rosidl_runtime_c/type_hash.h"
#include "rosidl_runtime_c/visibility_control.h"
#include "avoa3d/msg/rosidl_generator_c__visibility_control.h"

#include "avoa3d/msg/detail/element_characteristics_stamped__struct.h"

/// Initialize msg/ElementCharacteristicsStamped message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * avoa3d__msg__ElementCharacteristicsStamped
 * )) before or use
 * avoa3d__msg__ElementCharacteristicsStamped__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_avoa3d
bool
avoa3d__msg__ElementCharacteristicsStamped__init(avoa3d__msg__ElementCharacteristicsStamped * msg);

/// Finalize msg/ElementCharacteristicsStamped message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_avoa3d
void
avoa3d__msg__ElementCharacteristicsStamped__fini(avoa3d__msg__ElementCharacteristicsStamped * msg);

/// Create msg/ElementCharacteristicsStamped message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * avoa3d__msg__ElementCharacteristicsStamped__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_avoa3d
avoa3d__msg__ElementCharacteristicsStamped *
avoa3d__msg__ElementCharacteristicsStamped__create(void);

/// Destroy msg/ElementCharacteristicsStamped message.
/**
 * It calls
 * avoa3d__msg__ElementCharacteristicsStamped__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_avoa3d
void
avoa3d__msg__ElementCharacteristicsStamped__destroy(avoa3d__msg__ElementCharacteristicsStamped * msg);

/// Check for msg/ElementCharacteristicsStamped message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_avoa3d
bool
avoa3d__msg__ElementCharacteristicsStamped__are_equal(const avoa3d__msg__ElementCharacteristicsStamped * lhs, const avoa3d__msg__ElementCharacteristicsStamped * rhs);

/// Copy a msg/ElementCharacteristicsStamped message.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source message pointer.
 * \param[out] output The target message pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer is null
 *   or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_avoa3d
bool
avoa3d__msg__ElementCharacteristicsStamped__copy(
  const avoa3d__msg__ElementCharacteristicsStamped * input,
  avoa3d__msg__ElementCharacteristicsStamped * output);

/// Retrieve pointer to the hash of the description of this type.
ROSIDL_GENERATOR_C_PUBLIC_avoa3d
const rosidl_type_hash_t *
avoa3d__msg__ElementCharacteristicsStamped__get_type_hash(
  const rosidl_message_type_support_t * type_support);

/// Retrieve pointer to the description of this type.
ROSIDL_GENERATOR_C_PUBLIC_avoa3d
const rosidl_runtime_c__type_description__TypeDescription *
avoa3d__msg__ElementCharacteristicsStamped__get_type_description(
  const rosidl_message_type_support_t * type_support);

/// Retrieve pointer to the single raw source text that defined this type.
ROSIDL_GENERATOR_C_PUBLIC_avoa3d
const rosidl_runtime_c__type_description__TypeSource *
avoa3d__msg__ElementCharacteristicsStamped__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support);

/// Retrieve pointer to the recursive raw sources that defined the description of this type.
ROSIDL_GENERATOR_C_PUBLIC_avoa3d
const rosidl_runtime_c__type_description__TypeSource__Sequence *
avoa3d__msg__ElementCharacteristicsStamped__get_type_description_sources(
  const rosidl_message_type_support_t * type_support);

/// Initialize array of msg/ElementCharacteristicsStamped messages.
/**
 * It allocates the memory for the number of elements and calls
 * avoa3d__msg__ElementCharacteristicsStamped__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_avoa3d
bool
avoa3d__msg__ElementCharacteristicsStamped__Sequence__init(avoa3d__msg__ElementCharacteristicsStamped__Sequence * array, size_t size);

/// Finalize array of msg/ElementCharacteristicsStamped messages.
/**
 * It calls
 * avoa3d__msg__ElementCharacteristicsStamped__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_avoa3d
void
avoa3d__msg__ElementCharacteristicsStamped__Sequence__fini(avoa3d__msg__ElementCharacteristicsStamped__Sequence * array);

/// Create array of msg/ElementCharacteristicsStamped messages.
/**
 * It allocates the memory for the array and calls
 * avoa3d__msg__ElementCharacteristicsStamped__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_avoa3d
avoa3d__msg__ElementCharacteristicsStamped__Sequence *
avoa3d__msg__ElementCharacteristicsStamped__Sequence__create(size_t size);

/// Destroy array of msg/ElementCharacteristicsStamped messages.
/**
 * It calls
 * avoa3d__msg__ElementCharacteristicsStamped__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_avoa3d
void
avoa3d__msg__ElementCharacteristicsStamped__Sequence__destroy(avoa3d__msg__ElementCharacteristicsStamped__Sequence * array);

/// Check for msg/ElementCharacteristicsStamped message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_avoa3d
bool
avoa3d__msg__ElementCharacteristicsStamped__Sequence__are_equal(const avoa3d__msg__ElementCharacteristicsStamped__Sequence * lhs, const avoa3d__msg__ElementCharacteristicsStamped__Sequence * rhs);

/// Copy an array of msg/ElementCharacteristicsStamped messages.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source array pointer.
 * \param[out] output The target array pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer
 *   is null or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_avoa3d
bool
avoa3d__msg__ElementCharacteristicsStamped__Sequence__copy(
  const avoa3d__msg__ElementCharacteristicsStamped__Sequence * input,
  avoa3d__msg__ElementCharacteristicsStamped__Sequence * output);

#ifdef __cplusplus
}
#endif

#endif  // AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__FUNCTIONS_H_
