// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from robot_msgs:msg/OrderEvent.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__ORDER_EVENT__STRUCT_HPP_
#define ROBOT_MSGS__MSG__DETAIL__ORDER_EVENT__STRUCT_HPP_

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
# define DEPRECATED__robot_msgs__msg__OrderEvent __attribute__((deprecated))
#else
# define DEPRECATED__robot_msgs__msg__OrderEvent __declspec(deprecated)
#endif

namespace robot_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct OrderEvent_
{
  using Type = OrderEvent_<ContainerAllocator>;

  explicit OrderEvent_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->order_id = "";
      this->tablet_id = "";
      this->type = 0;
      this->target_lat = 0.0;
      this->target_lon = 0.0;
      this->payload_json = "";
    }
  }

  explicit OrderEvent_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_alloc, _init),
    order_id(_alloc),
    tablet_id(_alloc),
    payload_json(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->order_id = "";
      this->tablet_id = "";
      this->type = 0;
      this->target_lat = 0.0;
      this->target_lon = 0.0;
      this->payload_json = "";
    }
  }

  // field types and members
  using _header_type =
    std_msgs::msg::Header_<ContainerAllocator>;
  _header_type header;
  using _order_id_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _order_id_type order_id;
  using _tablet_id_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _tablet_id_type tablet_id;
  using _type_type =
    uint8_t;
  _type_type type;
  using _target_lat_type =
    double;
  _target_lat_type target_lat;
  using _target_lon_type =
    double;
  _target_lon_type target_lon;
  using _payload_json_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _payload_json_type payload_json;

  // setters for named parameter idiom
  Type & set__header(
    const std_msgs::msg::Header_<ContainerAllocator> & _arg)
  {
    this->header = _arg;
    return *this;
  }
  Type & set__order_id(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->order_id = _arg;
    return *this;
  }
  Type & set__tablet_id(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->tablet_id = _arg;
    return *this;
  }
  Type & set__type(
    const uint8_t & _arg)
  {
    this->type = _arg;
    return *this;
  }
  Type & set__target_lat(
    const double & _arg)
  {
    this->target_lat = _arg;
    return *this;
  }
  Type & set__target_lon(
    const double & _arg)
  {
    this->target_lon = _arg;
    return *this;
  }
  Type & set__payload_json(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->payload_json = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    robot_msgs::msg::OrderEvent_<ContainerAllocator> *;
  using ConstRawPtr =
    const robot_msgs::msg::OrderEvent_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<robot_msgs::msg::OrderEvent_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<robot_msgs::msg::OrderEvent_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      robot_msgs::msg::OrderEvent_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<robot_msgs::msg::OrderEvent_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      robot_msgs::msg::OrderEvent_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<robot_msgs::msg::OrderEvent_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<robot_msgs::msg::OrderEvent_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<robot_msgs::msg::OrderEvent_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__robot_msgs__msg__OrderEvent
    std::shared_ptr<robot_msgs::msg::OrderEvent_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__robot_msgs__msg__OrderEvent
    std::shared_ptr<robot_msgs::msg::OrderEvent_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const OrderEvent_ & other) const
  {
    if (this->header != other.header) {
      return false;
    }
    if (this->order_id != other.order_id) {
      return false;
    }
    if (this->tablet_id != other.tablet_id) {
      return false;
    }
    if (this->type != other.type) {
      return false;
    }
    if (this->target_lat != other.target_lat) {
      return false;
    }
    if (this->target_lon != other.target_lon) {
      return false;
    }
    if (this->payload_json != other.payload_json) {
      return false;
    }
    return true;
  }
  bool operator!=(const OrderEvent_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct OrderEvent_

// alias to use template instance with default allocator
using OrderEvent =
  robot_msgs::msg::OrderEvent_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace robot_msgs

#endif  // ROBOT_MSGS__MSG__DETAIL__ORDER_EVENT__STRUCT_HPP_
