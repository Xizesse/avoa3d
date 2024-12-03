// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from avoa:msg/ElementCharacteristicsStamped.idl
// generated code does not contain a copyright notice

#ifndef AVOA__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__STRUCT_HPP_
#define AVOA__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.hpp"
// Member 'pose'
#include "geometry_msgs/msg/detail/pose__struct.hpp"
// Member 'velocity'
// Member 'size'
#include "geometry_msgs/msg/detail/vector3__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__avoa__msg__ElementCharacteristicsStamped __attribute__((deprecated))
#else
# define DEPRECATED__avoa__msg__ElementCharacteristicsStamped __declspec(deprecated)
#endif

namespace avoa
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct ElementCharacteristicsStamped_
{
  using Type = ElementCharacteristicsStamped_<ContainerAllocator>;

  explicit ElementCharacteristicsStamped_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_init),
    pose(_init),
    velocity(_init),
    size(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->id = 0l;
      this->type = 0l;
      this->dynamic = false;
      this->protective_zone = 0.0f;
    }
  }

  explicit ElementCharacteristicsStamped_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_alloc, _init),
    pose(_alloc, _init),
    velocity(_alloc, _init),
    size(_alloc, _init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->id = 0l;
      this->type = 0l;
      this->dynamic = false;
      this->protective_zone = 0.0f;
    }
  }

  // field types and members
  using _header_type =
    std_msgs::msg::Header_<ContainerAllocator>;
  _header_type header;
  using _id_type =
    int32_t;
  _id_type id;
  using _type_type =
    int32_t;
  _type_type type;
  using _dynamic_type =
    bool;
  _dynamic_type dynamic;
  using _pose_type =
    geometry_msgs::msg::Pose_<ContainerAllocator>;
  _pose_type pose;
  using _velocity_type =
    geometry_msgs::msg::Vector3_<ContainerAllocator>;
  _velocity_type velocity;
  using _size_type =
    geometry_msgs::msg::Vector3_<ContainerAllocator>;
  _size_type size;
  using _protective_zone_type =
    float;
  _protective_zone_type protective_zone;

  // setters for named parameter idiom
  Type & set__header(
    const std_msgs::msg::Header_<ContainerAllocator> & _arg)
  {
    this->header = _arg;
    return *this;
  }
  Type & set__id(
    const int32_t & _arg)
  {
    this->id = _arg;
    return *this;
  }
  Type & set__type(
    const int32_t & _arg)
  {
    this->type = _arg;
    return *this;
  }
  Type & set__dynamic(
    const bool & _arg)
  {
    this->dynamic = _arg;
    return *this;
  }
  Type & set__pose(
    const geometry_msgs::msg::Pose_<ContainerAllocator> & _arg)
  {
    this->pose = _arg;
    return *this;
  }
  Type & set__velocity(
    const geometry_msgs::msg::Vector3_<ContainerAllocator> & _arg)
  {
    this->velocity = _arg;
    return *this;
  }
  Type & set__size(
    const geometry_msgs::msg::Vector3_<ContainerAllocator> & _arg)
  {
    this->size = _arg;
    return *this;
  }
  Type & set__protective_zone(
    const float & _arg)
  {
    this->protective_zone = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    avoa::msg::ElementCharacteristicsStamped_<ContainerAllocator> *;
  using ConstRawPtr =
    const avoa::msg::ElementCharacteristicsStamped_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<avoa::msg::ElementCharacteristicsStamped_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<avoa::msg::ElementCharacteristicsStamped_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      avoa::msg::ElementCharacteristicsStamped_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<avoa::msg::ElementCharacteristicsStamped_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      avoa::msg::ElementCharacteristicsStamped_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<avoa::msg::ElementCharacteristicsStamped_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<avoa::msg::ElementCharacteristicsStamped_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<avoa::msg::ElementCharacteristicsStamped_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__avoa__msg__ElementCharacteristicsStamped
    std::shared_ptr<avoa::msg::ElementCharacteristicsStamped_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__avoa__msg__ElementCharacteristicsStamped
    std::shared_ptr<avoa::msg::ElementCharacteristicsStamped_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const ElementCharacteristicsStamped_ & other) const
  {
    if (this->header != other.header) {
      return false;
    }
    if (this->id != other.id) {
      return false;
    }
    if (this->type != other.type) {
      return false;
    }
    if (this->dynamic != other.dynamic) {
      return false;
    }
    if (this->pose != other.pose) {
      return false;
    }
    if (this->velocity != other.velocity) {
      return false;
    }
    if (this->size != other.size) {
      return false;
    }
    if (this->protective_zone != other.protective_zone) {
      return false;
    }
    return true;
  }
  bool operator!=(const ElementCharacteristicsStamped_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct ElementCharacteristicsStamped_

// alias to use template instance with default allocator
using ElementCharacteristicsStamped =
  avoa::msg::ElementCharacteristicsStamped_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace avoa

#endif  // AVOA__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__STRUCT_HPP_
