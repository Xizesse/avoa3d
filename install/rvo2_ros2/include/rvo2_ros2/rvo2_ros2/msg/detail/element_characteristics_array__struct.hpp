// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from rvo2_ros2:msg/ElementCharacteristicsArray.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "rvo2_ros2/msg/element_characteristics_array.hpp"


#ifndef RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__STRUCT_HPP_
#define RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'elements'
#include "rvo2_ros2/msg/detail/element_characteristics_stamped__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__rvo2_ros2__msg__ElementCharacteristicsArray __attribute__((deprecated))
#else
# define DEPRECATED__rvo2_ros2__msg__ElementCharacteristicsArray __declspec(deprecated)
#endif

namespace rvo2_ros2
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct ElementCharacteristicsArray_
{
  using Type = ElementCharacteristicsArray_<ContainerAllocator>;

  explicit ElementCharacteristicsArray_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
  }

  explicit ElementCharacteristicsArray_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
    (void)_alloc;
  }

  // field types and members
  using _elements_type =
    std::vector<rvo2_ros2::msg::ElementCharacteristicsStamped_<ContainerAllocator>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<rvo2_ros2::msg::ElementCharacteristicsStamped_<ContainerAllocator>>>;
  _elements_type elements;

  // setters for named parameter idiom
  Type & set__elements(
    const std::vector<rvo2_ros2::msg::ElementCharacteristicsStamped_<ContainerAllocator>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<rvo2_ros2::msg::ElementCharacteristicsStamped_<ContainerAllocator>>> & _arg)
  {
    this->elements = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    rvo2_ros2::msg::ElementCharacteristicsArray_<ContainerAllocator> *;
  using ConstRawPtr =
    const rvo2_ros2::msg::ElementCharacteristicsArray_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<rvo2_ros2::msg::ElementCharacteristicsArray_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<rvo2_ros2::msg::ElementCharacteristicsArray_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      rvo2_ros2::msg::ElementCharacteristicsArray_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<rvo2_ros2::msg::ElementCharacteristicsArray_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      rvo2_ros2::msg::ElementCharacteristicsArray_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<rvo2_ros2::msg::ElementCharacteristicsArray_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<rvo2_ros2::msg::ElementCharacteristicsArray_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<rvo2_ros2::msg::ElementCharacteristicsArray_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__rvo2_ros2__msg__ElementCharacteristicsArray
    std::shared_ptr<rvo2_ros2::msg::ElementCharacteristicsArray_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__rvo2_ros2__msg__ElementCharacteristicsArray
    std::shared_ptr<rvo2_ros2::msg::ElementCharacteristicsArray_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const ElementCharacteristicsArray_ & other) const
  {
    if (this->elements != other.elements) {
      return false;
    }
    return true;
  }
  bool operator!=(const ElementCharacteristicsArray_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct ElementCharacteristicsArray_

// alias to use template instance with default allocator
using ElementCharacteristicsArray =
  rvo2_ros2::msg::ElementCharacteristicsArray_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace rvo2_ros2

#endif  // RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__STRUCT_HPP_
