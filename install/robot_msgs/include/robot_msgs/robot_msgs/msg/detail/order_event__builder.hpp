// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from robot_msgs:msg/OrderEvent.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__ORDER_EVENT__BUILDER_HPP_
#define ROBOT_MSGS__MSG__DETAIL__ORDER_EVENT__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "robot_msgs/msg/detail/order_event__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace robot_msgs
{

namespace msg
{

namespace builder
{

class Init_OrderEvent_payload_json
{
public:
  explicit Init_OrderEvent_payload_json(::robot_msgs::msg::OrderEvent & msg)
  : msg_(msg)
  {}
  ::robot_msgs::msg::OrderEvent payload_json(::robot_msgs::msg::OrderEvent::_payload_json_type arg)
  {
    msg_.payload_json = std::move(arg);
    return std::move(msg_);
  }

private:
  ::robot_msgs::msg::OrderEvent msg_;
};

class Init_OrderEvent_target_lon
{
public:
  explicit Init_OrderEvent_target_lon(::robot_msgs::msg::OrderEvent & msg)
  : msg_(msg)
  {}
  Init_OrderEvent_payload_json target_lon(::robot_msgs::msg::OrderEvent::_target_lon_type arg)
  {
    msg_.target_lon = std::move(arg);
    return Init_OrderEvent_payload_json(msg_);
  }

private:
  ::robot_msgs::msg::OrderEvent msg_;
};

class Init_OrderEvent_target_lat
{
public:
  explicit Init_OrderEvent_target_lat(::robot_msgs::msg::OrderEvent & msg)
  : msg_(msg)
  {}
  Init_OrderEvent_target_lon target_lat(::robot_msgs::msg::OrderEvent::_target_lat_type arg)
  {
    msg_.target_lat = std::move(arg);
    return Init_OrderEvent_target_lon(msg_);
  }

private:
  ::robot_msgs::msg::OrderEvent msg_;
};

class Init_OrderEvent_type
{
public:
  explicit Init_OrderEvent_type(::robot_msgs::msg::OrderEvent & msg)
  : msg_(msg)
  {}
  Init_OrderEvent_target_lat type(::robot_msgs::msg::OrderEvent::_type_type arg)
  {
    msg_.type = std::move(arg);
    return Init_OrderEvent_target_lat(msg_);
  }

private:
  ::robot_msgs::msg::OrderEvent msg_;
};

class Init_OrderEvent_tablet_id
{
public:
  explicit Init_OrderEvent_tablet_id(::robot_msgs::msg::OrderEvent & msg)
  : msg_(msg)
  {}
  Init_OrderEvent_type tablet_id(::robot_msgs::msg::OrderEvent::_tablet_id_type arg)
  {
    msg_.tablet_id = std::move(arg);
    return Init_OrderEvent_type(msg_);
  }

private:
  ::robot_msgs::msg::OrderEvent msg_;
};

class Init_OrderEvent_order_id
{
public:
  explicit Init_OrderEvent_order_id(::robot_msgs::msg::OrderEvent & msg)
  : msg_(msg)
  {}
  Init_OrderEvent_tablet_id order_id(::robot_msgs::msg::OrderEvent::_order_id_type arg)
  {
    msg_.order_id = std::move(arg);
    return Init_OrderEvent_tablet_id(msg_);
  }

private:
  ::robot_msgs::msg::OrderEvent msg_;
};

class Init_OrderEvent_header
{
public:
  Init_OrderEvent_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_OrderEvent_order_id header(::robot_msgs::msg::OrderEvent::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_OrderEvent_order_id(msg_);
  }

private:
  ::robot_msgs::msg::OrderEvent msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::robot_msgs::msg::OrderEvent>()
{
  return robot_msgs::msg::builder::Init_OrderEvent_header();
}

}  // namespace robot_msgs

#endif  // ROBOT_MSGS__MSG__DETAIL__ORDER_EVENT__BUILDER_HPP_
