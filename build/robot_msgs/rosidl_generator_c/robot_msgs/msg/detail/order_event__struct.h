// NOLINT: This file starts with a BOM since it contain non-ASCII characters
// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from robot_msgs:msg/OrderEvent.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__ORDER_EVENT__STRUCT_H_
#define ROBOT_MSGS__MSG__DETAIL__ORDER_EVENT__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.h"
// Member 'order_id'
// Member 'tablet_id'
// Member 'payload_json'
#include "rosidl_runtime_c/string.h"

/// Struct defined in msg/OrderEvent in the package robot_msgs.
/**
  * 订单事件（平板 → 车端）
 */
typedef struct robot_msgs__msg__OrderEvent
{
  std_msgs__msg__Header header;
  rosidl_runtime_c__String order_id;
  rosidl_runtime_c__String tablet_id;
  /// 1=START_DELIVERY, 2=PAUSE, 3=RESUME, 4=CANCEL, 5=ARRIVED
  uint8_t type;
  double target_lat;
  double target_lon;
  /// 订单详情 JSON
  rosidl_runtime_c__String payload_json;
} robot_msgs__msg__OrderEvent;

// Struct for a sequence of robot_msgs__msg__OrderEvent.
typedef struct robot_msgs__msg__OrderEvent__Sequence
{
  robot_msgs__msg__OrderEvent * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} robot_msgs__msg__OrderEvent__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // ROBOT_MSGS__MSG__DETAIL__ORDER_EVENT__STRUCT_H_
