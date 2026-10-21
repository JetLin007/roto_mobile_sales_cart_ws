// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from robot_msgs:msg/ChassisState.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__CHASSIS_STATE__TRAITS_HPP_
#define ROBOT_MSGS__MSG__DETAIL__CHASSIS_STATE__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "robot_msgs/msg/detail/chassis_state__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__traits.hpp"

namespace robot_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const ChassisState & msg,
  std::ostream & out)
{
  out << "{";
  // member: header
  {
    out << "header: ";
    to_flow_style_yaml(msg.header, out);
    out << ", ";
  }

  // member: odom_x
  {
    out << "odom_x: ";
    rosidl_generator_traits::value_to_yaml(msg.odom_x, out);
    out << ", ";
  }

  // member: odom_y
  {
    out << "odom_y: ";
    rosidl_generator_traits::value_to_yaml(msg.odom_y, out);
    out << ", ";
  }

  // member: odom_theta
  {
    out << "odom_theta: ";
    rosidl_generator_traits::value_to_yaml(msg.odom_theta, out);
    out << ", ";
  }

  // member: vel_linear
  {
    out << "vel_linear: ";
    rosidl_generator_traits::value_to_yaml(msg.vel_linear, out);
    out << ", ";
  }

  // member: vel_angular
  {
    out << "vel_angular: ";
    rosidl_generator_traits::value_to_yaml(msg.vel_angular, out);
    out << ", ";
  }

  // member: state
  {
    out << "state: ";
    rosidl_generator_traits::value_to_yaml(msg.state, out);
    out << ", ";
  }

  // member: ultrasonic_front
  {
    out << "ultrasonic_front: ";
    rosidl_generator_traits::value_to_yaml(msg.ultrasonic_front, out);
    out << ", ";
  }

  // member: ultrasonic_rear
  {
    out << "ultrasonic_rear: ";
    rosidl_generator_traits::value_to_yaml(msg.ultrasonic_rear, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const ChassisState & msg,
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

  // member: odom_x
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "odom_x: ";
    rosidl_generator_traits::value_to_yaml(msg.odom_x, out);
    out << "\n";
  }

  // member: odom_y
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "odom_y: ";
    rosidl_generator_traits::value_to_yaml(msg.odom_y, out);
    out << "\n";
  }

  // member: odom_theta
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "odom_theta: ";
    rosidl_generator_traits::value_to_yaml(msg.odom_theta, out);
    out << "\n";
  }

  // member: vel_linear
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "vel_linear: ";
    rosidl_generator_traits::value_to_yaml(msg.vel_linear, out);
    out << "\n";
  }

  // member: vel_angular
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "vel_angular: ";
    rosidl_generator_traits::value_to_yaml(msg.vel_angular, out);
    out << "\n";
  }

  // member: state
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "state: ";
    rosidl_generator_traits::value_to_yaml(msg.state, out);
    out << "\n";
  }

  // member: ultrasonic_front
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "ultrasonic_front: ";
    rosidl_generator_traits::value_to_yaml(msg.ultrasonic_front, out);
    out << "\n";
  }

  // member: ultrasonic_rear
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "ultrasonic_rear: ";
    rosidl_generator_traits::value_to_yaml(msg.ultrasonic_rear, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const ChassisState & msg, bool use_flow_style = false)
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
  const robot_msgs::msg::ChassisState & msg,
  std::ostream & out, size_t indentation = 0)
{
  robot_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use robot_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const robot_msgs::msg::ChassisState & msg)
{
  return robot_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<robot_msgs::msg::ChassisState>()
{
  return "robot_msgs::msg::ChassisState";
}

template<>
inline const char * name<robot_msgs::msg::ChassisState>()
{
  return "robot_msgs/msg/ChassisState";
}

template<>
struct has_fixed_size<robot_msgs::msg::ChassisState>
  : std::integral_constant<bool, has_fixed_size<std_msgs::msg::Header>::value> {};

template<>
struct has_bounded_size<robot_msgs::msg::ChassisState>
  : std::integral_constant<bool, has_bounded_size<std_msgs::msg::Header>::value> {};

template<>
struct is_message<robot_msgs::msg::ChassisState>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // ROBOT_MSGS__MSG__DETAIL__CHASSIS_STATE__TRAITS_HPP_
