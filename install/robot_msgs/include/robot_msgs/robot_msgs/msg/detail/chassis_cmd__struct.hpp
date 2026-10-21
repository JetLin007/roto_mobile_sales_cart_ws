// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from robot_msgs:msg/ChassisCmd.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__CHASSIS_CMD__STRUCT_HPP_
#define ROBOT_MSGS__MSG__DETAIL__CHASSIS_CMD__STRUCT_HPP_

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
# define DEPRECATED__robot_msgs__msg__ChassisCmd __attribute__((deprecated))
#else
# define DEPRECATED__robot_msgs__msg__ChassisCmd __declspec(deprecated)
#endif

namespace robot_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct ChassisCmd_
{
  using Type = ChassisCmd_<ContainerAllocator>;

  explicit ChassisCmd_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->linear_x = 0.0f;
      this->linear_y = 0.0f;
      this->angular_z = 0.0f;
      this->gear = 0;
    }
  }

  explicit ChassisCmd_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_alloc, _init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->linear_x = 0.0f;
      this->linear_y = 0.0f;
      this->angular_z = 0.0f;
      this->gear = 0;
    }
  }

  // field types and members
  using _header_type =
    std_msgs::msg::Header_<ContainerAllocator>;
  _header_type header;
  using _linear_x_type =
    float;
  _linear_x_type linear_x;
  using _linear_y_type =
    float;
  _linear_y_type linear_y;
  using _angular_z_type =
    float;
  _angular_z_type angular_z;
  using _gear_type =
    uint8_t;
  _gear_type gear;

  // setters for named parameter idiom
  Type & set__header(
    const std_msgs::msg::Header_<ContainerAllocator> & _arg)
  {
    this->header = _arg;
    return *this;
  }
  Type & set__linear_x(
    const float & _arg)
  {
    this->linear_x = _arg;
    return *this;
  }
  Type & set__linear_y(
    const float & _arg)
  {
    this->linear_y = _arg;
    return *this;
  }
  Type & set__angular_z(
    const float & _arg)
  {
    this->angular_z = _arg;
    return *this;
  }
  Type & set__gear(
    const uint8_t & _arg)
  {
    this->gear = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    robot_msgs::msg::ChassisCmd_<ContainerAllocator> *;
  using ConstRawPtr =
    const robot_msgs::msg::ChassisCmd_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<robot_msgs::msg::ChassisCmd_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<robot_msgs::msg::ChassisCmd_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      robot_msgs::msg::ChassisCmd_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<robot_msgs::msg::ChassisCmd_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      robot_msgs::msg::ChassisCmd_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<robot_msgs::msg::ChassisCmd_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<robot_msgs::msg::ChassisCmd_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<robot_msgs::msg::ChassisCmd_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__robot_msgs__msg__ChassisCmd
    std::shared_ptr<robot_msgs::msg::ChassisCmd_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__robot_msgs__msg__ChassisCmd
    std::shared_ptr<robot_msgs::msg::ChassisCmd_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const ChassisCmd_ & other) const
  {
    if (this->header != other.header) {
      return false;
    }
    if (this->linear_x != other.linear_x) {
      return false;
    }
    if (this->linear_y != other.linear_y) {
      return false;
    }
    if (this->angular_z != other.angular_z) {
      return false;
    }
    if (this->gear != other.gear) {
      return false;
    }
    return true;
  }
  bool operator!=(const ChassisCmd_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct ChassisCmd_

// alias to use template instance with default allocator
using ChassisCmd =
  robot_msgs::msg::ChassisCmd_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace robot_msgs

#endif  // ROBOT_MSGS__MSG__DETAIL__CHASSIS_CMD__STRUCT_HPP_
