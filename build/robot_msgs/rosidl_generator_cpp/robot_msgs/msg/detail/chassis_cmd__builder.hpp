// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from robot_msgs:msg/ChassisCmd.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__CHASSIS_CMD__BUILDER_HPP_
#define ROBOT_MSGS__MSG__DETAIL__CHASSIS_CMD__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "robot_msgs/msg/detail/chassis_cmd__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace robot_msgs
{

namespace msg
{

namespace builder
{

class Init_ChassisCmd_gear
{
public:
  explicit Init_ChassisCmd_gear(::robot_msgs::msg::ChassisCmd & msg)
  : msg_(msg)
  {}
  ::robot_msgs::msg::ChassisCmd gear(::robot_msgs::msg::ChassisCmd::_gear_type arg)
  {
    msg_.gear = std::move(arg);
    return std::move(msg_);
  }

private:
  ::robot_msgs::msg::ChassisCmd msg_;
};

class Init_ChassisCmd_angular_z
{
public:
  explicit Init_ChassisCmd_angular_z(::robot_msgs::msg::ChassisCmd & msg)
  : msg_(msg)
  {}
  Init_ChassisCmd_gear angular_z(::robot_msgs::msg::ChassisCmd::_angular_z_type arg)
  {
    msg_.angular_z = std::move(arg);
    return Init_ChassisCmd_gear(msg_);
  }

private:
  ::robot_msgs::msg::ChassisCmd msg_;
};

class Init_ChassisCmd_linear_y
{
public:
  explicit Init_ChassisCmd_linear_y(::robot_msgs::msg::ChassisCmd & msg)
  : msg_(msg)
  {}
  Init_ChassisCmd_angular_z linear_y(::robot_msgs::msg::ChassisCmd::_linear_y_type arg)
  {
    msg_.linear_y = std::move(arg);
    return Init_ChassisCmd_angular_z(msg_);
  }

private:
  ::robot_msgs::msg::ChassisCmd msg_;
};

class Init_ChassisCmd_linear_x
{
public:
  explicit Init_ChassisCmd_linear_x(::robot_msgs::msg::ChassisCmd & msg)
  : msg_(msg)
  {}
  Init_ChassisCmd_linear_y linear_x(::robot_msgs::msg::ChassisCmd::_linear_x_type arg)
  {
    msg_.linear_x = std::move(arg);
    return Init_ChassisCmd_linear_y(msg_);
  }

private:
  ::robot_msgs::msg::ChassisCmd msg_;
};

class Init_ChassisCmd_header
{
public:
  Init_ChassisCmd_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_ChassisCmd_linear_x header(::robot_msgs::msg::ChassisCmd::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_ChassisCmd_linear_x(msg_);
  }

private:
  ::robot_msgs::msg::ChassisCmd msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::robot_msgs::msg::ChassisCmd>()
{
  return robot_msgs::msg::builder::Init_ChassisCmd_header();
}

}  // namespace robot_msgs

#endif  // ROBOT_MSGS__MSG__DETAIL__CHASSIS_CMD__BUILDER_HPP_
