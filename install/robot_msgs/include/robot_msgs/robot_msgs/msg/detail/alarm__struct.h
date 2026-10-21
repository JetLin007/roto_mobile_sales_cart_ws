// NOLINT: This file starts with a BOM since it contain non-ASCII characters
// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from robot_msgs:msg/Alarm.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__ALARM__STRUCT_H_
#define ROBOT_MSGS__MSG__DETAIL__ALARM__STRUCT_H_

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
// Member 'source'
// Member 'message'
#include "rosidl_runtime_c/string.h"

/// Struct defined in msg/Alarm in the package robot_msgs.
/**
  * 告警消息
 */
typedef struct robot_msgs__msg__Alarm
{
  std_msgs__msg__Header header;
  /// 告警码
  uint16_t alarm_code;
  /// 1=INFO 2=WARN 3=ERROR 4=FATAL
  uint8_t level;
  /// 触发源 (e.g. "driver_bms")
  rosidl_runtime_c__String source;
  rosidl_runtime_c__String message;
  bool cleared;
} robot_msgs__msg__Alarm;

// Struct for a sequence of robot_msgs__msg__Alarm.
typedef struct robot_msgs__msg__Alarm__Sequence
{
  robot_msgs__msg__Alarm * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} robot_msgs__msg__Alarm__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // ROBOT_MSGS__MSG__DETAIL__ALARM__STRUCT_H_
