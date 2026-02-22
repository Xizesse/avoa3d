// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from rvo2_ros2:msg/ElementCharacteristicsArray.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "rvo2_ros2/msg/element_characteristics_array.hpp"


#ifndef RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__BUILDER_HPP_
#define RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "rvo2_ros2/msg/detail/element_characteristics_array__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace rvo2_ros2
{

namespace msg
{

namespace builder
{

class Init_ElementCharacteristicsArray_elements
{
public:
  Init_ElementCharacteristicsArray_elements()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::rvo2_ros2::msg::ElementCharacteristicsArray elements(::rvo2_ros2::msg::ElementCharacteristicsArray::_elements_type arg)
  {
    msg_.elements = std::move(arg);
    return std::move(msg_);
  }

private:
  ::rvo2_ros2::msg::ElementCharacteristicsArray msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::rvo2_ros2::msg::ElementCharacteristicsArray>()
{
  return rvo2_ros2::msg::builder::Init_ElementCharacteristicsArray_elements();
}

}  // namespace rvo2_ros2

#endif  // RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__BUILDER_HPP_
