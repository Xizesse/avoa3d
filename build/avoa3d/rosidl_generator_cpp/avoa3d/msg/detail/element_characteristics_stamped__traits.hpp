// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from avoa3d:msg/ElementCharacteristicsStamped.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "avoa3d/msg/element_characteristics_stamped.hpp"


#ifndef AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__TRAITS_HPP_
#define AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "avoa3d/msg/detail/element_characteristics_stamped__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__traits.hpp"
// Member 'pose'
#include "geometry_msgs/msg/detail/pose__traits.hpp"
// Member 'velocity'
// Member 'size'
#include "geometry_msgs/msg/detail/vector3__traits.hpp"

namespace avoa3d
{

namespace msg
{

inline void to_flow_style_yaml(
  const ElementCharacteristicsStamped & msg,
  std::ostream & out)
{
  out << "{";
  // member: header
  {
    out << "header: ";
    to_flow_style_yaml(msg.header, out);
    out << ", ";
  }

  // member: id
  {
    out << "id: ";
    rosidl_generator_traits::value_to_yaml(msg.id, out);
    out << ", ";
  }

  // member: type
  {
    out << "type: ";
    rosidl_generator_traits::value_to_yaml(msg.type, out);
    out << ", ";
  }

  // member: dynamic
  {
    out << "dynamic: ";
    rosidl_generator_traits::value_to_yaml(msg.dynamic, out);
    out << ", ";
  }

  // member: pose
  {
    out << "pose: ";
    to_flow_style_yaml(msg.pose, out);
    out << ", ";
  }

  // member: position_x_std
  {
    out << "position_x_std: ";
    rosidl_generator_traits::value_to_yaml(msg.position_x_std, out);
    out << ", ";
  }

  // member: position_y_std
  {
    out << "position_y_std: ";
    rosidl_generator_traits::value_to_yaml(msg.position_y_std, out);
    out << ", ";
  }

  // member: position_z_std
  {
    out << "position_z_std: ";
    rosidl_generator_traits::value_to_yaml(msg.position_z_std, out);
    out << ", ";
  }

  // member: velocity
  {
    out << "velocity: ";
    to_flow_style_yaml(msg.velocity, out);
    out << ", ";
  }

  // member: velocity_x_std
  {
    out << "velocity_x_std: ";
    rosidl_generator_traits::value_to_yaml(msg.velocity_x_std, out);
    out << ", ";
  }

  // member: velocity_y_std
  {
    out << "velocity_y_std: ";
    rosidl_generator_traits::value_to_yaml(msg.velocity_y_std, out);
    out << ", ";
  }

  // member: velocity_z_std
  {
    out << "velocity_z_std: ";
    rosidl_generator_traits::value_to_yaml(msg.velocity_z_std, out);
    out << ", ";
  }

  // member: size
  {
    out << "size: ";
    to_flow_style_yaml(msg.size, out);
    out << ", ";
  }

  // member: radius_std
  {
    out << "radius_std: ";
    rosidl_generator_traits::value_to_yaml(msg.radius_std, out);
    out << ", ";
  }

  // member: protective_zone
  {
    out << "protective_zone: ";
    rosidl_generator_traits::value_to_yaml(msg.protective_zone, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const ElementCharacteristicsStamped & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: header
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "header:\n";
    to_block_style_yaml(msg.header, out, indentation + 2);
  }

  // member: id
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "id: ";
    rosidl_generator_traits::value_to_yaml(msg.id, out);
    out << "\n";
  }

  // member: type
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "type: ";
    rosidl_generator_traits::value_to_yaml(msg.type, out);
    out << "\n";
  }

  // member: dynamic
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "dynamic: ";
    rosidl_generator_traits::value_to_yaml(msg.dynamic, out);
    out << "\n";
  }

  // member: pose
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "pose:\n";
    to_block_style_yaml(msg.pose, out, indentation + 2);
  }

  // member: position_x_std
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "position_x_std: ";
    rosidl_generator_traits::value_to_yaml(msg.position_x_std, out);
    out << "\n";
  }

  // member: position_y_std
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "position_y_std: ";
    rosidl_generator_traits::value_to_yaml(msg.position_y_std, out);
    out << "\n";
  }

  // member: position_z_std
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "position_z_std: ";
    rosidl_generator_traits::value_to_yaml(msg.position_z_std, out);
    out << "\n";
  }

  // member: velocity
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "velocity:\n";
    to_block_style_yaml(msg.velocity, out, indentation + 2);
  }

  // member: velocity_x_std
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "velocity_x_std: ";
    rosidl_generator_traits::value_to_yaml(msg.velocity_x_std, out);
    out << "\n";
  }

  // member: velocity_y_std
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "velocity_y_std: ";
    rosidl_generator_traits::value_to_yaml(msg.velocity_y_std, out);
    out << "\n";
  }

  // member: velocity_z_std
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "velocity_z_std: ";
    rosidl_generator_traits::value_to_yaml(msg.velocity_z_std, out);
    out << "\n";
  }

  // member: size
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "size:\n";
    to_block_style_yaml(msg.size, out, indentation + 2);
  }

  // member: radius_std
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "radius_std: ";
    rosidl_generator_traits::value_to_yaml(msg.radius_std, out);
    out << "\n";
  }

  // member: protective_zone
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "protective_zone: ";
    rosidl_generator_traits::value_to_yaml(msg.protective_zone, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const ElementCharacteristicsStamped & msg, bool use_flow_style = false)
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
  const avoa3d::msg::ElementCharacteristicsStamped & msg,
  std::ostream & out, size_t indentation = 0)
{
  avoa3d::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use avoa3d::msg::to_yaml() instead")]]
inline std::string to_yaml(const avoa3d::msg::ElementCharacteristicsStamped & msg)
{
  return avoa3d::msg::to_yaml(msg);
}

template<>
inline const char * data_type<avoa3d::msg::ElementCharacteristicsStamped>()
{
  return "avoa3d::msg::ElementCharacteristicsStamped";
}

template<>
inline const char * name<avoa3d::msg::ElementCharacteristicsStamped>()
{
  return "avoa3d/msg/ElementCharacteristicsStamped";
}

template<>
struct has_fixed_size<avoa3d::msg::ElementCharacteristicsStamped>
  : std::integral_constant<bool, has_fixed_size<geometry_msgs::msg::Pose>::value && has_fixed_size<geometry_msgs::msg::Vector3>::value && has_fixed_size<std_msgs::msg::Header>::value> {};

template<>
struct has_bounded_size<avoa3d::msg::ElementCharacteristicsStamped>
  : std::integral_constant<bool, has_bounded_size<geometry_msgs::msg::Pose>::value && has_bounded_size<geometry_msgs::msg::Vector3>::value && has_bounded_size<std_msgs::msg::Header>::value> {};

template<>
struct is_message<avoa3d::msg::ElementCharacteristicsStamped>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__TRAITS_HPP_
