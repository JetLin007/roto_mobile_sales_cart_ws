// NOLINT: This file starts with a BOM since it contain non-ASCII characters
// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from robot_msgs:msg/BmsStatus.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__BMS_STATUS__STRUCT_H_
#define ROBOT_MSGS__MSG__DETAIL__BMS_STATUS__STRUCT_H_

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

/// Struct defined in msg/BmsStatus in the package robot_msgs.
/**
  * 电池管理系统状态
 */
typedef struct robot_msgs__msg__BmsStatus
{
  std_msgs__msg__Header header;
  /// 剩余电量 %
  uint8_t soc;
  /// mV
  int16_t voltage;
  /// mA, 正=放电 负=充电
  int16_t current;
  /// 0.1°C
  int16_t temp;
  bool charging;
  /// 0~100 %
  uint8_t health;
} robot_msgs__msg__BmsStatus;

// Struct for a sequence of robot_msgs__msg__BmsStatus.
typedef struct robot_msgs__msg__BmsStatus__Sequence
{
  robot_msgs__msg__BmsStatus * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} robot_msgs__msg__BmsStatus__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // ROBOT_MSGS__MSG__DETAIL__BMS_STATUS__STRUCT_H_
