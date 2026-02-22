// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from rvo2_ros2:msg/ElementCharacteristicsArray.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "rvo2_ros2/msg/element_characteristics_array.hpp"


#ifndef RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__TRAITS_HPP_
#define RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "rvo2_ros2/msg/detail/element_characteristics_array__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'elements'
#include "rvo2_ros2/msg/detail/element_characteristics_stamped__traits.hpp"

namespace rvo2_ros2
{

namespace msg
{

inline void to_flow_style_yaml(
  const ElementCharacteristicsArray & msg,
  std::ostream & out)
{
  out << "{";
  // member: elements
  {
    if (msg.elements.size() == 0) {
      out << "elements: []";
    } else {
      out << "elements: [";
      size_t pending_items = msg.elements.size();
      for (auto item : msg.elements) {
        to_flow_style_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const ElementCharacteristicsArray & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: elements
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.elements.size() == 0) {
      out << "elements: []\n";
    } else {
      out << "elements:\n";
      for (auto item : msg.elements) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "-\n";
        to_block_style_yaml(item, out, indentation + 2);
      }
    }
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const ElementCharacteristicsArray & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace rvo2_ros2

namespace rosidl_generator_traits
{

[[deprecated("use rvo2_ros2::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const rvo2_ros2::msg::ElementCharacteristicsArray & msg,
  std::ostream & out, size_t indentation = 0)
{
  rvo2_ros2::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use rvo2_ros2::msg::to_yaml() instead")]]
inline std::string to_yaml(const rvo2_ros2::msg::ElementCharacteristicsArray & msg)
{
  return rvo2_ros2::msg::to_yaml(msg);
}

template<>
inline const char * data_type<rvo2_ros2::msg::ElementCharacteristicsArray>()
{
  return "rvo2_ros2::msg::ElementCharacteristicsArray";
}

template<>
inline const char * name<rvo2_ros2::msg::ElementCharacteristicsArray>()
{
  return "rvo2_ros2/msg/ElementCharacteristicsArray";
}

template<>
struct has_fixed_size<rvo2_ros2::msg::ElementCharacteristicsArray>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<rvo2_ros2::msg::ElementCharacteristicsArray>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<rvo2_ros2::msg::ElementCharacteristicsArray>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__TRAITS_HPP_
