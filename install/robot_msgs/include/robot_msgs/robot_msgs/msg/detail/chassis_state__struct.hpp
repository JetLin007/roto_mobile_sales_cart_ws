// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from robot_msgs:msg/ChassisState.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__CHASSIS_STATE__STRUCT_HPP_
#define ROBOT_MSGS__MSG__DETAIL__CHASSIS_STATE__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__robot_msgs__msg__ChassisState __attribute__((deprecated))
#else
# define DEPRECATED__robot_msgs__msg__ChassisState __declspec(deprecated)
#endif

namespace robot_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct ChassisState_
{
  using Type = ChassisState_<ContainerAllocator>;

  explicit ChassisState_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->odom_x = 0.0f;
      this->odom_y = 0.0f;
      this->odom_theta = 0.0f;
      this->vel_linear = 0.0f;
      this->vel_angular = 0.0f;
      this->state = 0;
      this->ultrasonic_front = 0;
      this->ultrasonic_rear = 0;
    }
  }

  explicit ChassisState_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_alloc, _init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->odom_x = 0.0f;
      this->odom_y = 0.0f;
      this->odom_theta = 0.0f;
      this->vel_linear = 0.0f;
      this->vel_angular = 0.0f;
      this->state = 0;
      this->ultrasonic_front = 0;
      this->ultrasonic_rear = 0;
    }
  }

  // field types and members
  using _header_type =
    std_msgs::msg::Header_<ContainerAllocator>;
  _header_type header;
  using _odom_x_type =
    float;
  _odom_x_type odom_x;
  using _odom_y_type =
    float;
  _odom_y_type odom_y;
  using _odom_theta_type =
    float;
  _odom_theta_type odom_theta;
  using _vel_linear_type =
    float;
  _vel_linear_type vel_linear;
  using _vel_angular_type =
    float;
  _vel_angular_type vel_angular;
  using _state_type =
    uint8_t;
  _state_type state;
  using _ultrasonic_front_type =
    uint8_t;
  _ultrasonic_front_type ultrasonic_front;
  using _ultrasonic_rear_type =
    uint8_t;
  _ultrasonic_rear_type ultrasonic_rear;

  // setters for named parameter idiom
  Type & set__header(
    const std_msgs::msg::Header_<ContainerAllocator> & _arg)
  {
    this->header = _arg;
    return *this;
  }
  Type & set__odom_x(
    const float & _arg)
  {
    this->odom_x = _arg;
    return *this;
  }
  Type & set__odom_y(
    const float & _arg)
  {
    this->odom_y = _arg;
    return *this;
  }
  Type & set__odom_theta(
    const float & _arg)
  {
    this->odom_theta = _arg;
    return *this;
  }
  Type & set__vel_linear(
    const float & _arg)
  {
    this->vel_linear = _arg;
    return *this;
  }
  Type & set__vel_angular(
    const float & _arg)
  {
    this->vel_angular = _arg;
    return *this;
  }
  Type & set__state(
    const uint8_t & _arg)
  {
    this->state = _arg;
    return *this;
  }
  Type & set__ultrasonic_front(
    const uint8_t & _arg)
  {
    this->ultrasonic_front = _arg;
    return *this;
  }
  Type & set__ultrasonic_rear(
    const uint8_t & _arg)
  {
    this->ultrasonic_rear = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    robot_msgs::msg::ChassisState_<ContainerAllocator> *;
  using ConstRawPtr =
    const robot_msgs::msg::ChassisState_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<robot_msgs::msg::ChassisState_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<robot_msgs::msg::ChassisState_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      robot_msgs::msg::ChassisState_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<robot_msgs::msg::ChassisState_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      robot_msgs::msg::ChassisState_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<robot_msgs::msg::ChassisState_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<robot_msgs::msg::ChassisState_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<robot_msgs::msg::ChassisState_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__robot_msgs__msg__ChassisState
    std::shared_ptr<robot_msgs::msg::ChassisState_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__robot_msgs__msg__ChassisState
    std::shared_ptr<robot_msgs::msg::ChassisState_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const ChassisState_ & other) const
  {
    if (this->header != other.header) {
      return false;
    }
    if (this->odom_x != other.odom_x) {
      return false;
    }
    if (this->odom_y != other.odom_y) {
      return false;
    }
    if (this->odom_theta != other.odom_theta) {
      return false;
    }
    if (this->vel_linear != other.vel_linear) {
      return false;
    }
    if (this->vel_angular != other.vel_angular) {
      return false;
    }
    if (this->state != other.state) {
      return false;
    }
    if (this->ultrasonic_front != other.ultrasonic_front) {
      return false;
    }
    if (this->ultrasonic_rear != other.ultrasonic_rear) {
      return false;
    }
    return true;
  }
  bool operator!=(const ChassisState_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct ChassisState_

// alias to use template instance with default allocator
using ChassisState =
  robot_msgs::msg::ChassisState_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace robot_msgs

#endif  // ROBOT_MSGS__MSG__DETAIL__CHASSIS_STATE__STRUCT_HPP_
