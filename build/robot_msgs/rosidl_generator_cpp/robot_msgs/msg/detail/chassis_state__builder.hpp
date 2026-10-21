// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from robot_msgs:msg/ChassisState.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__CHASSIS_STATE__BUILDER_HPP_
#define ROBOT_MSGS__MSG__DETAIL__CHASSIS_STATE__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "robot_msgs/msg/detail/chassis_state__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace robot_msgs
{

namespace msg
{

namespace builder
{

class Init_ChassisState_ultrasonic_rear
{
public:
  explicit Init_ChassisState_ultrasonic_rear(::robot_msgs::msg::ChassisState & msg)
  : msg_(msg)
  {}
  ::robot_msgs::msg::ChassisState ultrasonic_rear(::robot_msgs::msg::ChassisState::_ultrasonic_rear_type arg)
  {
    msg_.ultrasonic_rear = std::move(arg);
    return std::move(msg_);
  }

private:
  ::robot_msgs::msg::ChassisState msg_;
};

class Init_ChassisState_ultrasonic_front
{
public:
  explicit Init_ChassisState_ultrasonic_front(::robot_msgs::msg::ChassisState & msg)
  : msg_(msg)
  {}
  Init_ChassisState_ultrasonic_rear ultrasonic_front(::robot_msgs::msg::ChassisState::_ultrasonic_front_type arg)
  {
    msg_.ultrasonic_front = std::move(arg);
    return Init_ChassisState_ultrasonic_rear(msg_);
  }

private:
  ::robot_msgs::msg::ChassisState msg_;
};

class Init_ChassisState_state
{
public:
  explicit Init_ChassisState_state(::robot_msgs::msg::ChassisState & msg)
  : msg_(msg)
  {}
  Init_ChassisState_ultrasonic_front state(::robot_msgs::msg::ChassisState::_state_type arg)
  {
    msg_.state = std::move(arg);
    return Init_ChassisState_ultrasonic_front(msg_);
  }

private:
  ::robot_msgs::msg::ChassisState msg_;
};

class Init_ChassisState_vel_angular
{
public:
  explicit Init_ChassisState_vel_angular(::robot_msgs::msg::ChassisState & msg)
  : msg_(msg)
  {}
  Init_ChassisState_state vel_angular(::robot_msgs::msg::ChassisState::_vel_angular_type arg)
  {
    msg_.vel_angular = std::move(arg);
    return Init_ChassisState_state(msg_);
  }

private:
  ::robot_msgs::msg::ChassisState msg_;
};

class Init_ChassisState_vel_linear
{
public:
  explicit Init_ChassisState_vel_linear(::robot_msgs::msg::ChassisState & msg)
  : msg_(msg)
  {}
  Init_ChassisState_vel_angular vel_linear(::robot_msgs::msg::ChassisState::_vel_linear_type arg)
  {
    msg_.vel_linear = std::move(arg);
    return Init_ChassisState_vel_angular(msg_);
  }

private:
  ::robot_msgs::msg::ChassisState msg_;
};

class Init_ChassisState_odom_theta
{
public:
  explicit Init_ChassisState_odom_theta(::robot_msgs::msg::ChassisState & msg)
  : msg_(msg)
  {}
  Init_ChassisState_vel_linear odom_theta(::robot_msgs::msg::ChassisState::_odom_theta_type arg)
  {
    msg_.odom_theta = std::move(arg);
    return Init_ChassisState_vel_linear(msg_);
  }

private:
  ::robot_msgs::msg::ChassisState msg_;
};

class Init_ChassisState_odom_y
{
public:
  explicit Init_ChassisState_odom_y(::robot_msgs::msg::ChassisState & msg)
  : msg_(msg)
  {}
  Init_ChassisState_odom_theta odom_y(::robot_msgs::msg::ChassisState::_odom_y_type arg)
  {
    msg_.odom_y = std::move(arg);
    return Init_ChassisState_odom_theta(msg_);
  }

private:
  ::robot_msgs::msg::ChassisState msg_;
};

class Init_ChassisState_odom_x
{
public:
  explicit Init_ChassisState_odom_x(::robot_msgs::msg::ChassisState & msg)
  : msg_(msg)
  {}
  Init_ChassisState_odom_y odom_x(::robot_msgs::msg::ChassisState::_odom_x_type arg)
  {
    msg_.odom_x = std::move(arg);
    return Init_ChassisState_odom_y(msg_);
  }

private:
  ::robot_msgs::msg::ChassisState msg_;
};

class Init_ChassisState_header
{
public:
  Init_ChassisState_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_ChassisState_odom_x header(::robot_msgs::msg::ChassisState::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_ChassisState_odom_x(msg_);
  }

private:
  ::robot_msgs::msg::ChassisState msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::robot_msgs::msg::ChassisState>()
{
  return robot_msgs::msg::builder::Init_ChassisState_header();
}

}  // namespace robot_msgs

#endif  // ROBOT_MSGS__MSG__DETAIL__CHASSIS_STATE__BUILDER_HPP_
