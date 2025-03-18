// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from avoa3d:msg/ElementCharacteristicsArray.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "avoa3d/msg/element_characteristics_array.hpp"


#ifndef AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__TRAITS_HPP_
#define AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "avoa3d/msg/detail/element_characteristics_array__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'elements'
#include "avoa3d/msg/detail/element_characteristics_stamped__traits.hpp"

namespace avoa3d
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

}  // namespace avoa3d

namespace rosidl_generator_traits
{

[[deprecated("use avoa3d::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const avoa3d::msg::ElementCharacteristicsArray & msg,
  std::ostream & out, size_t indentation = 0)
{
  avoa3d::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use avoa3d::msg::to_yaml() instead")]]
inline std::string to_yaml(const avoa3d::msg::ElementCharacteristicsArray & msg)
{
  return avoa3d::msg::to_yaml(msg);
}

template<>
inline const char * data_type<avoa3d::msg::ElementCharacteristicsArray>()
{
  return "avoa3d::msg::ElementCharacteristicsArray";
}

template<>
inline const char * name<avoa3d::msg::ElementCharacteristicsArray>()
{
  return "avoa3d/msg/ElementCharacteristicsArray";
}

template<>
struct has_fixed_size<avoa3d::msg::ElementCharacteristicsArray>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<avoa3d::msg::ElementCharacteristicsArray>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<avoa3d::msg::ElementCharacteristicsArray>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_ARRAY__TRAITS_HPP_
