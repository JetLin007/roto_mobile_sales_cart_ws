// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from robot_msgs:msg/BmsStatus.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__BMS_STATUS__BUILDER_HPP_
#define ROBOT_MSGS__MSG__DETAIL__BMS_STATUS__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "robot_msgs/msg/detail/bms_status__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace robot_msgs
{

namespace msg
{

namespace builder
{

class Init_BmsStatus_health
{
public:
  explicit Init_BmsStatus_health(::robot_msgs::msg::BmsStatus & msg)
  : msg_(msg)
  {}
  ::robot_msgs::msg::BmsStatus health(::robot_msgs::msg::BmsStatus::_health_type arg)
  {
    msg_.health = std::move(arg);
    return std::move(msg_);
  }

private:
  ::robot_msgs::msg::BmsStatus msg_;
};

class Init_BmsStatus_charging
{
public:
  explicit Init_BmsStatus_charging(::robot_msgs::msg::BmsStatus & msg)
  : msg_(msg)
  {}
  Init_BmsStatus_health charging(::robot_msgs::msg::BmsStatus::_charging_type arg)
  {
    msg_.charging = std::move(arg);
    return Init_BmsStatus_health(msg_);
  }

private:
  ::robot_msgs::msg::BmsStatus msg_;
};

class Init_BmsStatus_temp
{
public:
  explicit Init_BmsStatus_temp(::robot_msgs::msg::BmsStatus & msg)
  : msg_(msg)
  {}
  Init_BmsStatus_charging temp(::robot_msgs::msg::BmsStatus::_temp_type arg)
  {
    msg_.temp = std::move(arg);
    return Init_BmsStatus_charging(msg_);
  }

private:
  ::robot_msgs::msg::BmsStatus msg_;
};

class Init_BmsStatus_current
{
public:
  explicit Init_BmsStatus_current(::robot_msgs::msg::BmsStatus & msg)
  : msg_(msg)
  {}
  Init_BmsStatus_temp current(::robot_msgs::msg::BmsStatus::_current_type arg)
  {
    msg_.current = std::move(arg);
    return Init_BmsStatus_temp(msg_);
  }

private:
  ::robot_msgs::msg::BmsStatus msg_;
};

class Init_BmsStatus_voltage
{
public:
  explicit Init_BmsStatus_voltage(::robot_msgs::msg::BmsStatus & msg)
  : msg_(msg)
  {}
  Init_BmsStatus_current voltage(::robot_msgs::msg::BmsStatus::_voltage_type arg)
  {
    msg_.voltage = std::move(arg);
    return Init_BmsStatus_current(msg_);
  }

private:
  ::robot_msgs::msg::BmsStatus msg_;
};

class Init_BmsStatus_soc
{
public:
  explicit Init_BmsStatus_soc(::robot_msgs::msg::BmsStatus & msg)
  : msg_(msg)
  {}
  Init_BmsStatus_voltage soc(::robot_msgs::msg::BmsStatus::_soc_type arg)
  {
    msg_.soc = std::move(arg);
    return Init_BmsStatus_voltage(msg_);
  }

private:
  ::robot_msgs::msg::BmsStatus msg_;
};

class Init_BmsStatus_header
{
public:
  Init_BmsStatus_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_BmsStatus_soc header(::robot_msgs::msg::BmsStatus::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_BmsStatus_soc(msg_);
  }

private:
  ::robot_msgs::msg::BmsStatus msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::robot_msgs::msg::BmsStatus>()
{
  return robot_msgs::msg::builder::Init_BmsStatus_header();
}

}  // namespace robot_msgs

#endif  // ROBOT_MSGS__MSG__DETAIL__BMS_STATUS__BUILDER_HPP_
