// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from avoa3d:msg/ElementCharacteristicsStamped.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "avoa3d/msg/element_characteristics_stamped.hpp"


#ifndef AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__BUILDER_HPP_
#define AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "avoa3d/msg/detail/element_characteristics_stamped__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace avoa3d
{

namespace msg
{

namespace builder
{

class Init_ElementCharacteristicsStamped_protective_zone
{
public:
  explicit Init_ElementCharacteristicsStamped_protective_zone(::avoa3d::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  ::avoa3d::msg::ElementCharacteristicsStamped protective_zone(::avoa3d::msg::ElementCharacteristicsStamped::_protective_zone_type arg)
  {
    msg_.protective_zone = std::move(arg);
    return std::move(msg_);
  }

private:
  ::avoa3d::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_radius_std
{
public:
  explicit Init_ElementCharacteristicsStamped_radius_std(::avoa3d::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_protective_zone radius_std(::avoa3d::msg::ElementCharacteristicsStamped::_radius_std_type arg)
  {
    msg_.radius_std = std::move(arg);
    return Init_ElementCharacteristicsStamped_protective_zone(msg_);
  }

private:
  ::avoa3d::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_size
{
public:
  explicit Init_ElementCharacteristicsStamped_size(::avoa3d::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_radius_std size(::avoa3d::msg::ElementCharacteristicsStamped::_size_type arg)
  {
    msg_.size = std::move(arg);
    return Init_ElementCharacteristicsStamped_radius_std(msg_);
  }

private:
  ::avoa3d::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_velocity_z_std
{
public:
  explicit Init_ElementCharacteristicsStamped_velocity_z_std(::avoa3d::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_size velocity_z_std(::avoa3d::msg::ElementCharacteristicsStamped::_velocity_z_std_type arg)
  {
    msg_.velocity_z_std = std::move(arg);
    return Init_ElementCharacteristicsStamped_size(msg_);
  }

private:
  ::avoa3d::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_velocity_y_std
{
public:
  explicit Init_ElementCharacteristicsStamped_velocity_y_std(::avoa3d::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_velocity_z_std velocity_y_std(::avoa3d::msg::ElementCharacteristicsStamped::_velocity_y_std_type arg)
  {
    msg_.velocity_y_std = std::move(arg);
    return Init_ElementCharacteristicsStamped_velocity_z_std(msg_);
  }

private:
  ::avoa3d::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_velocity_x_std
{
public:
  explicit Init_ElementCharacteristicsStamped_velocity_x_std(::avoa3d::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_velocity_y_std velocity_x_std(::avoa3d::msg::ElementCharacteristicsStamped::_velocity_x_std_type arg)
  {
    msg_.velocity_x_std = std::move(arg);
    return Init_ElementCharacteristicsStamped_velocity_y_std(msg_);
  }

private:
  ::avoa3d::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_velocity
{
public:
  explicit Init_ElementCharacteristicsStamped_velocity(::avoa3d::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_velocity_x_std velocity(::avoa3d::msg::ElementCharacteristicsStamped::_velocity_type arg)
  {
    msg_.velocity = std::move(arg);
    return Init_ElementCharacteristicsStamped_velocity_x_std(msg_);
  }

private:
  ::avoa3d::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_position_z_std
{
public:
  explicit Init_ElementCharacteristicsStamped_position_z_std(::avoa3d::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_velocity position_z_std(::avoa3d::msg::ElementCharacteristicsStamped::_position_z_std_type arg)
  {
    msg_.position_z_std = std::move(arg);
    return Init_ElementCharacteristicsStamped_velocity(msg_);
  }

private:
  ::avoa3d::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_position_y_std
{
public:
  explicit Init_ElementCharacteristicsStamped_position_y_std(::avoa3d::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_position_z_std position_y_std(::avoa3d::msg::ElementCharacteristicsStamped::_position_y_std_type arg)
  {
    msg_.position_y_std = std::move(arg);
    return Init_ElementCharacteristicsStamped_position_z_std(msg_);
  }

private:
  ::avoa3d::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_position_x_std
{
public:
  explicit Init_ElementCharacteristicsStamped_position_x_std(::avoa3d::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_position_y_std position_x_std(::avoa3d::msg::ElementCharacteristicsStamped::_position_x_std_type arg)
  {
    msg_.position_x_std = std::move(arg);
    return Init_ElementCharacteristicsStamped_position_y_std(msg_);
  }

private:
  ::avoa3d::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_pose
{
public:
  explicit Init_ElementCharacteristicsStamped_pose(::avoa3d::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_position_x_std pose(::avoa3d::msg::ElementCharacteristicsStamped::_pose_type arg)
  {
    msg_.pose = std::move(arg);
    return Init_ElementCharacteristicsStamped_position_x_std(msg_);
  }

private:
  ::avoa3d::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_dynamic
{
public:
  explicit Init_ElementCharacteristicsStamped_dynamic(::avoa3d::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_pose dynamic(::avoa3d::msg::ElementCharacteristicsStamped::_dynamic_type arg)
  {
    msg_.dynamic = std::move(arg);
    return Init_ElementCharacteristicsStamped_pose(msg_);
  }

private:
  ::avoa3d::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_type
{
public:
  explicit Init_ElementCharacteristicsStamped_type(::avoa3d::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_dynamic type(::avoa3d::msg::ElementCharacteristicsStamped::_type_type arg)
  {
    msg_.type = std::move(arg);
    return Init_ElementCharacteristicsStamped_dynamic(msg_);
  }

private:
  ::avoa3d::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_id
{
public:
  explicit Init_ElementCharacteristicsStamped_id(::avoa3d::msg::ElementCharacteristicsStamped & msg)
  : msg_(msg)
  {}
  Init_ElementCharacteristicsStamped_type id(::avoa3d::msg::ElementCharacteristicsStamped::_id_type arg)
  {
    msg_.id = std::move(arg);
    return Init_ElementCharacteristicsStamped_type(msg_);
  }

private:
  ::avoa3d::msg::ElementCharacteristicsStamped msg_;
};

class Init_ElementCharacteristicsStamped_header
{
public:
  Init_ElementCharacteristicsStamped_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_ElementCharacteristicsStamped_id header(::avoa3d::msg::ElementCharacteristicsStamped::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_ElementCharacteristicsStamped_id(msg_);
  }

private:
  ::avoa3d::msg::ElementCharacteristicsStamped msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::avoa3d::msg::ElementCharacteristicsStamped>()
{
  return avoa3d::msg::builder::Init_ElementCharacteristicsStamped_header();
}

}  // namespace avoa3d

#endif  // AVOA3D__MSG__DETAIL__ELEMENT_CHARACTERISTICS_STAMPED__BUILDER_HPP_
