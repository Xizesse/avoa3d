// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from rvo2_ros2:msg/ElementCharacteristicsStamped.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "rvo2_ros2/msg/element_characteristics_stamped.hpp"


#ifndef RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__BUILDER_HPP_
#define RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "rvo2_ros2/msg/detail/element_characteristics_stamped__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace rvo2_ros2
{

namespace msg
{

namespace builder
{

class Init_ElementCharacteristicsStamped_protective_zone
{
public:
  explicit Init_ElementCharacteristicsStamped_protective_zone(::rvo2_ros2::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  ::rvo2_ros2::msg::ElementCharacteristicsStamped protective_zone(::rvo2_ros2::msg::ElementCharacteristicsStamped::_protective_zone_type arg)
  {
    msg_.protective_zone = std::move(arg);
    return std::move(msg_);
  }

private:
  ::rvo2_ros2::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_radius_std
{
public:
  explicit Init_ElementCharacteristicsStamped_radius_std(::rvo2_ros2::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_protective_zone radius_std(::rvo2_ros2::msg::ElementCharacteristicsStamped::_radius_std_type arg)
  {
    msg_.radius_std = std::move(arg);
    return Init_ElementCharacteristicsStamped_protective_zone(msg_);
  }

private:
  ::rvo2_ros2::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_size
{
public:
  explicit Init_ElementCharacteristicsStamped_size(::rvo2_ros2::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_radius_std size(::rvo2_ros2::msg::ElementCharacteristicsStamped::_size_type arg)
  {
    msg_.size = std::move(arg);
    return Init_ElementCharacteristicsStamped_radius_std(msg_);
  }

private:
  ::rvo2_ros2::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_velocity_z_std
{
public:
  explicit Init_ElementCharacteristicsStamped_velocity_z_std(::rvo2_ros2::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_size velocity_z_std(::rvo2_ros2::msg::ElementCharacteristicsStamped::_velocity_z_std_type arg)
  {
    msg_.velocity_z_std = std::move(arg);
    return Init_ElementCharacteristicsStamped_size(msg_);
  }

private:
  ::rvo2_ros2::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_velocity_y_std
{
public:
  explicit Init_ElementCharacteristicsStamped_velocity_y_std(::rvo2_ros2::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_velocity_z_std velocity_y_std(::rvo2_ros2::msg::ElementCharacteristicsStamped::_velocity_y_std_type arg)
  {
    msg_.velocity_y_std = std::move(arg);
    return Init_ElementCharacteristicsStamped_velocity_z_std(msg_);
  }

private:
  ::rvo2_ros2::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_velocity_x_std
{
public:
  explicit Init_ElementCharacteristicsStamped_velocity_x_std(::rvo2_ros2::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_velocity_y_std velocity_x_std(::rvo2_ros2::msg::ElementCharacteristicsStamped::_velocity_x_std_type arg)
  {
    msg_.velocity_x_std = std::move(arg);
    return Init_ElementCharacteristicsStamped_velocity_y_std(msg_);
  }

private:
  ::rvo2_ros2::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_velocity
{
public:
  explicit Init_ElementCharacteristicsStamped_velocity(::rvo2_ros2::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_velocity_x_std velocity(::rvo2_ros2::msg::ElementCharacteristicsStamped::_velocity_type arg)
  {
    msg_.velocity = std::move(arg);
    return Init_ElementCharacteristicsStamped_velocity_x_std(msg_);
  }

private:
  ::rvo2_ros2::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_position_z_std
{
public:
  explicit Init_ElementCharacteristicsStamped_position_z_std(::rvo2_ros2::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_velocity position_z_std(::rvo2_ros2::msg::ElementCharacteristicsStamped::_position_z_std_type arg)
  {
    msg_.position_z_std = std::move(arg);
    return Init_ElementCharacteristicsStamped_velocity(msg_);
  }

private:
  ::rvo2_ros2::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_position_y_std
{
public:
  explicit Init_ElementCharacteristicsStamped_position_y_std(::rvo2_ros2::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_position_z_std position_y_std(::rvo2_ros2::msg::ElementCharacteristicsStamped::_position_y_std_type arg)
  {
    msg_.position_y_std = std::move(arg);
    return Init_ElementCharacteristicsStamped_position_z_std(msg_);
  }

private:
  ::rvo2_ros2::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_position_x_std
{
public:
  explicit Init_ElementCharacteristicsStamped_position_x_std(::rvo2_ros2::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_position_y_std position_x_std(::rvo2_ros2::msg::ElementCharacteristicsStamped::_position_x_std_type arg)
  {
    msg_.position_x_std = std::move(arg);
    return Init_ElementCharacteristicsStamped_position_y_std(msg_);
  }

private:
  ::rvo2_ros2::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_pose
{
public:
  explicit Init_ElementCharacteristicsStamped_pose(::rvo2_ros2::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_position_x_std pose(::rvo2_ros2::msg::ElementCharacteristicsStamped::_pose_type arg)
  {
    msg_.pose = std::move(arg);
    return Init_ElementCharacteristicsStamped_position_x_std(msg_);
  }

private:
  ::rvo2_ros2::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_dynamic
{
public:
  explicit Init_ElementCharacteristicsStamped_dynamic(::rvo2_ros2::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_pose dynamic(::rvo2_ros2::msg::ElementCharacteristicsStamped::_dynamic_type arg)
  {
    msg_.dynamic = std::move(arg);
    return Init_ElementCharacteristicsStamped_pose(msg_);
  }

private:
  ::rvo2_ros2::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_type
{
public:
  explicit Init_ElementCharacteristicsStamped_type(::rvo2_ros2::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_dynamic type(::rvo2_ros2::msg::ElementCharacteristicsStamped::_type_type arg)
  {
    msg_.type = std::move(arg);
    return Init_ElementCharacteristicsStamped_dynamic(msg_);
  }

private:
  ::rvo2_ros2::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_id
{
public:
  explicit Init_ElementCharacteristicsStamped_id(::rvo2_ros2::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_type id(::rvo2_ros2::msg::ElementCharacteristicsStamped::_id_type arg)
  {
    msg_.id = std::move(arg);
    return Init_ElementCharacteristicsStamped_type(msg_);
  }

private:
  ::rvo2_ros2::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_header
{
public:
  Init_ElementCharacteristicsStamped_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_ElementCharacteristicsStamped_id header(::rvo2_ros2::msg::ElementCharacteristicsStamped::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_ElementCharacteristicsStamped_id(msg_);
  }

private:
  ::rvo2_ros2::msg::ElementCharacteristicsStamped msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::rvo2_ros2::msg::ElementCharacteristicsStamped>()
{
  return rvo2_ros2::msg::builder::Init_ElementCharacteristicsStamped_header();
}

}  // namespace rvo2_ros2

#endif  // RVO2_ROS2__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__BUILDER_HPP_
