// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from robot_msgs:msg/GpsFix.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__GPS_FIX__BUILDER_HPP_
#define ROBOT_MSGS__MSG__DETAIL__GPS_FIX__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "robot_msgs/msg/detail/gps_fix__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace robot_msgs
{

namespace msg
{

namespace builder
{

class Init_GpsFix_vdop
{
public:
  explicit Init_GpsFix_vdop(::robot_msgs::msg::GpsFix & msg)
  : msg_(msg)
  {}
  ::robot_msgs::msg::GpsFix vdop(::robot_msgs::msg::GpsFix::_vdop_type arg)
  {
    msg_.vdop = std::move(arg);
    return std::move(msg_);
  }

private:
  ::robot_msgs::msg::GpsFix msg_;
};

class Init_GpsFix_hdop
{
public:
  explicit Init_GpsFix_hdop(::robot_msgs::msg::GpsFix & msg)
  : msg_(msg)
  {}
  Init_GpsFix_vdop hdop(::robot_msgs::msg::GpsFix::_hdop_type arg)
  {
    msg_.hdop = std::move(arg);
    return Init_GpsFix_vdop(msg_);
  }

private:
  ::robot_msgs::msg::GpsFix msg_;
};

class Init_GpsFix_satellites
{
public:
  explicit Init_GpsFix_satellites(::robot_msgs::msg::GpsFix & msg)
  : msg_(msg)
  {}
  Init_GpsFix_hdop satellites(::robot_msgs::msg::GpsFix::_satellites_type arg)
  {
    msg_.satellites = std::move(arg);
    return Init_GpsFix_hdop(msg_);
  }

private:
  ::robot_msgs::msg::GpsFix msg_;
};

class Init_GpsFix_fix_type
{
public:
  explicit Init_GpsFix_fix_type(::robot_msgs::msg::GpsFix & msg)
  : msg_(msg)
  {}
  Init_GpsFix_satellites fix_type(::robot_msgs::msg::GpsFix::_fix_type_type arg)
  {
    msg_.fix_type = std::move(arg);
    return Init_GpsFix_satellites(msg_);
  }

private:
  ::robot_msgs::msg::GpsFix msg_;
};

class Init_GpsFix_heading
{
public:
  explicit Init_GpsFix_heading(::robot_msgs::msg::GpsFix & msg)
  : msg_(msg)
  {}
  Init_GpsFix_fix_type heading(::robot_msgs::msg::GpsFix::_heading_type arg)
  {
    msg_.heading = std::move(arg);
    return Init_GpsFix_fix_type(msg_);
  }

private:
  ::robot_msgs::msg::GpsFix msg_;
};

class Init_GpsFix_speed
{
public:
  explicit Init_GpsFix_speed(::robot_msgs::msg::GpsFix & msg)
  : msg_(msg)
  {}
  Init_GpsFix_heading speed(::robot_msgs::msg::GpsFix::_speed_type arg)
  {
    msg_.speed = std::move(arg);
    return Init_GpsFix_heading(msg_);
  }

private:
  ::robot_msgs::msg::GpsFix msg_;
};

class Init_GpsFix_altitude
{
public:
  explicit Init_GpsFix_altitude(::robot_msgs::msg::GpsFix & msg)
  : msg_(msg)
  {}
  Init_GpsFix_speed altitude(::robot_msgs::msg::GpsFix::_altitude_type arg)
  {
    msg_.altitude = std::move(arg);
    return Init_GpsFix_speed(msg_);
  }

private:
  ::robot_msgs::msg::GpsFix msg_;
};

class Init_GpsFix_longitude
{
public:
  explicit Init_GpsFix_longitude(::robot_msgs::msg::GpsFix & msg)
  : msg_(msg)
  {}
  Init_GpsFix_altitude longitude(::robot_msgs::msg::GpsFix::_longitude_type arg)
  {
    msg_.longitude = std::move(arg);
    return Init_GpsFix_altitude(msg_);
  }

private:
  ::robot_msgs::msg::GpsFix msg_;
};

class Init_GpsFix_latitude
{
public:
  explicit Init_GpsFix_latitude(::robot_msgs::msg::GpsFix & msg)
  : msg_(msg)
  {}
  Init_GpsFix_longitude latitude(::robot_msgs::msg::GpsFix::_latitude_type arg)
  {
    msg_.latitude = std::move(arg);
    return Init_GpsFix_longitude(msg_);
  }

private:
  ::robot_msgs::msg::GpsFix msg_;
};

class Init_GpsFix_header
{
public:
  Init_GpsFix_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_GpsFix_latitude header(::robot_msgs::msg::GpsFix::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_GpsFix_latitude(msg_);
  }

private:
  ::robot_msgs::msg::GpsFix msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::robot_msgs::msg::GpsFix>()
{
  return robot_msgs::msg::builder::Init_GpsFix_header();
}

}  // namespace robot_msgs

#endif  // ROBOT_MSGS__MSG__DETAIL__GPS_FIX__BUILDER_HPP_
