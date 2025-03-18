// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from avoa3d:msg/ElementCharacteristicsArray.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "avoa3d/msg/element_characteristics_array.hpp"


#ifndef AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__BUILDER_HPP_
#define AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "avoa3d/msg/detail/element_characteristics_array__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace avoa3d
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
  ::avoa3d::msg::ElementCharacteristicsArray elements(::avoa3d::msg::ElementCharacteristicsArray::_elements_type arg)
  {
    msg_.elements = std::move(arg);
    return std::move(msg_);
  }

private:
  ::avoa3d::msg::ElementCharacteristicsArray msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::avoa3d::msg::ElementCharacteristicsArray>()
{
  return avoa3d::msg::builder::Init_ElementCharacteristicsArray_elements();
}

}  // namespace avoa3d

#endif  // AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__BUILDER_HPP_
