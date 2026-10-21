// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from robot_msgs:msg/OrderEvent.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__ORDER_EVENT__TRAITS_HPP_
#define ROBOT_MSGS__MSG__DETAIL__ORDER_EVENT__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "robot_msgs/msg/detail/order_event__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__traits.hpp"

namespace robot_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const OrderEvent & msg,
  std::ostream & out)
{
  out << "{";
  // member: header
  {
    out << "header: ";
    to_flow_style_yaml(msg.header, out);
    out << ", ";
  }

  // member: order_id
  {
    out << "order_id: ";
    rosidl_generator_traits::value_to_yaml(msg.order_id, out);
    out << ", ";
  }

  // member: tablet_id
  {
    out << "tablet_id: ";
    rosidl_generator_traits::value_to_yaml(msg.tablet_id, out);
    out << ", ";
  }

  // member: type
  {
    out << "type: ";
    rosidl_generator_traits::value_to_yaml(msg.type, out);
    out << ", ";
  }

  // member: target_lat
  {
    out << "target_lat: ";
    rosidl_generator_traits::value_to_yaml(msg.target_lat, out);
    out << ", ";
  }

  // member: target_lon
  {
    out << "target_lon: ";
    rosidl_generator_traits::value_to_yaml(msg.target_lon, out);
    out << ", ";
  }

  // member: payload_json
  {
    out << "payload_json: ";
    rosidl_generator_traits::value_to_yaml(msg.payload_json, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const OrderEvent & msg,
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

  // member: order_id
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "order_id: ";
    rosidl_generator_traits::value_to_yaml(msg.order_id, out);
    out << "\n";
  }

  // member: tablet_id
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "tablet_id: ";
    rosidl_generator_traits::value_to_yaml(msg.tablet_id, out);
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

  // member: target_lat
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "target_lat: ";
    rosidl_generator_traits::value_to_yaml(msg.target_lat, out);
    out << "\n";
  }

  // member: target_lon
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "target_lon: ";
    rosidl_generator_traits::value_to_yaml(msg.target_lon, out);
    out << "\n";
  }

  // member: payload_json
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "payload_json: ";
    rosidl_generator_traits::value_to_yaml(msg.payload_json, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const OrderEvent & msg, bool use_flow_style = false)
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

}  // namespace robot_msgs

namespace rosidl_generator_traits
{

[[deprecated("use robot_msgs::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const robot_msgs::msg::OrderEvent & msg,
  std::ostream & out, size_t indentation = 0)
{
  robot_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use robot_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const robot_msgs::msg::OrderEvent & msg)
{
  return robot_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<robot_msgs::msg::OrderEvent>()
{
  return "robot_msgs::msg::OrderEvent";
}

template<>
inline const char * name<robot_msgs::msg::OrderEvent>()
{
  return "robot_msgs/msg/OrderEvent";
}

template<>
struct has_fixed_size<robot_msgs::msg::OrderEvent>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<robot_msgs::msg::OrderEvent>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<robot_msgs::msg::OrderEvent>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // ROBOT_MSGS__MSG__DETAIL__ORDER_EVENT__TRAITS_HPP_
