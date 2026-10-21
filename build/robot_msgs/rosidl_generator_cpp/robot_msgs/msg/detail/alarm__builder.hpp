// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from robot_msgs:msg/Alarm.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__ALARM__BUILDER_HPP_
#define ROBOT_MSGS__MSG__DETAIL__ALARM__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "robot_msgs/msg/detail/alarm__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace robot_msgs
{

namespace msg
{

namespace builder
{

class Init_Alarm_cleared
{
public:
  explicit Init_Alarm_cleared(::robot_msgs::msg::Alarm & msg)
  : msg_(msg)
  {}
  ::robot_msgs::msg::Alarm cleared(::robot_msgs::msg::Alarm::_cleared_type arg)
  {
    msg_.cleared = std::move(arg);
    return std::move(msg_);
  }

private:
  ::robot_msgs::msg::Alarm msg_;
};

class Init_Alarm_message
{
public:
  explicit Init_Alarm_message(::robot_msgs::msg::Alarm & msg)
  : msg_(msg)
  {}
  Init_Alarm_cleared message(::robot_msgs::msg::Alarm::_message_type arg)
  {
    msg_.message = std::move(arg);
    return Init_Alarm_cleared(msg_);
  }

private:
  ::robot_msgs::msg::Alarm msg_;
};

class Init_Alarm_source
{
public:
  explicit Init_Alarm_source(::robot_msgs::msg::Alarm & msg)
  : msg_(msg)
  {}
  Init_Alarm_message source(::robot_msgs::msg::Alarm::_source_type arg)
  {
    msg_.source = std::move(arg);
    return Init_Alarm_message(msg_);
  }

private:
  ::robot_msgs::msg::Alarm msg_;
};

class Init_Alarm_level
{
public:
  explicit Init_Alarm_level(::robot_msgs::msg::Alarm & msg)
  : msg_(msg)
  {}
  Init_Alarm_source level(::robot_msgs::msg::Alarm::_level_type arg)
  {
    msg_.level = std::move(arg);
    return Init_Alarm_source(msg_);
  }

private:
  ::robot_msgs::msg::Alarm msg_;
};

class Init_Alarm_alarm_code
{
public:
  explicit Init_Alarm_alarm_code(::robot_msgs::msg::Alarm & msg)
  : msg_(msg)
  {}
  Init_Alarm_level alarm_code(::robot_msgs::msg::Alarm::_alarm_code_type arg)
  {
    msg_.alarm_code = std::move(arg);
    return Init_Alarm_level(msg_);
  }

private:
  ::robot_msgs::msg::Alarm msg_;
};

class Init_Alarm_header
{
public:
  Init_Alarm_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Alarm_alarm_code header(::robot_msgs::msg::Alarm::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_Alarm_alarm_code(msg_);
  }

private:
  ::robot_msgs::msg::Alarm msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::robot_msgs::msg::Alarm>()
{
  return robot_msgs::msg::builder::Init_Alarm_header();
}

}  // namespace robot_msgs

#endif  // ROBOT_MSGS__MSG__DETAIL__ALARM__BUILDER_HPP_
