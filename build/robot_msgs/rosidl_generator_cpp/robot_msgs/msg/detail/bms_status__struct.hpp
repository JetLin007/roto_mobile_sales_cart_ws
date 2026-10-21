// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from robot_msgs:msg/BmsStatus.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__BMS_STATUS__STRUCT_HPP_
#define ROBOT_MSGS__MSG__DETAIL__BMS_STATUS__STRUCT_HPP_

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
# define DEPRECATED__robot_msgs__msg__BmsStatus __attribute__((deprecated))
#else
# define DEPRECATED__robot_msgs__msg__BmsStatus __declspec(deprecated)
#endif

namespace robot_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct BmsStatus_
{
  using Type = BmsStatus_<ContainerAllocator>;

  explicit BmsStatus_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->soc = 0;
      this->voltage = 0;
      this->current = 0;
      this->temp = 0;
      this->charging = false;
      this->health = 0;
    }
  }

  explicit BmsStatus_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_alloc, _init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->soc = 0;
      this->voltage = 0;
      this->current = 0;
      this->temp = 0;
      this->charging = false;
      this->health = 0;
    }
  }

  // field types and members
  using _header_type =
    std_msgs::msg::Header_<ContainerAllocator>;
  _header_type header;
  using _soc_type =
    uint8_t;
  _soc_type soc;
  using _voltage_type =
    int16_t;
  _voltage_type voltage;
  using _current_type =
    int16_t;
  _current_type current;
  using _temp_type =
    int16_t;
  _temp_type temp;
  using _charging_type =
    bool;
  _charging_type charging;
  using _health_type =
    uint8_t;
  _health_type health;

  // setters for named parameter idiom
  Type & set__header(
    const std_msgs::msg::Header_<ContainerAllocator> & _arg)
  {
    this->header = _arg;
    return *this;
  }
  Type & set__soc(
    const uint8_t & _arg)
  {
    this->soc = _arg;
    return *this;
  }
  Type & set__voltage(
    const int16_t & _arg)
  {
    this->voltage = _arg;
    return *this;
  }
  Type & set__current(
    const int16_t & _arg)
  {
    this->current = _arg;
    return *this;
  }
  Type & set__temp(
    const int16_t & _arg)
  {
    this->temp = _arg;
    return *this;
  }
  Type & set__charging(
    const bool & _arg)
  {
    this->charging = _arg;
    return *this;
  }
  Type & set__health(
    const uint8_t & _arg)
  {
    this->health = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    robot_msgs::msg::BmsStatus_<ContainerAllocator> *;
  using ConstRawPtr =
    const robot_msgs::msg::BmsStatus_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<robot_msgs::msg::BmsStatus_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<robot_msgs::msg::BmsStatus_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      robot_msgs::msg::BmsStatus_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<robot_msgs::msg::BmsStatus_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      robot_msgs::msg::BmsStatus_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<robot_msgs::msg::BmsStatus_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<robot_msgs::msg::BmsStatus_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<robot_msgs::msg::BmsStatus_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__robot_msgs__msg__BmsStatus
    std::shared_ptr<robot_msgs::msg::BmsStatus_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__robot_msgs__msg__BmsStatus
    std::shared_ptr<robot_msgs::msg::BmsStatus_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const BmsStatus_ & other) const
  {
    if (this->header != other.header) {
      return false;
    }
    if (this->soc != other.soc) {
      return false;
    }
    if (this->voltage != other.voltage) {
      return false;
    }
    if (this->current != other.current) {
      return false;
    }
    if (this->temp != other.temp) {
      return false;
    }
    if (this->charging != other.charging) {
      return false;
    }
    if (this->health != other.health) {
      return false;
    }
    return true;
  }
  bool operator!=(const BmsStatus_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct BmsStatus_

// alias to use template instance with default allocator
using BmsStatus =
  robot_msgs::msg::BmsStatus_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace robot_msgs

#endif  // ROBOT_MSGS__MSG__DETAIL__BMS_STATUS__STRUCT_HPP_
