// NOLINT: This file starts with a BOM since it contain non-ASCII characters
// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from robot_msgs:msg/ChassisCmd.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__CHASSIS_CMD__STRUCT_H_
#define ROBOT_MSGS__MSG__DETAIL__CHASSIS_CMD__STRUCT_H_

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

/// Struct defined in msg/ChassisCmd in the package robot_msgs.
/**
  * 底盘速度指令
 */
typedef struct robot_msgs__msg__ChassisCmd
{
  std_msgs__msg__Header header;
  /// m/s
  float linear_x;
  /// m/s (麦轮可用)
  float linear_y;
  /// rad/s
  float angular_z;
  /// 0=STOP 1=FORWARD 2=REVERSE
  uint8_t gear;
} robot_msgs__msg__ChassisCmd;

// Struct for a sequence of robot_msgs__msg__ChassisCmd.
typedef struct robot_msgs__msg__ChassisCmd__Sequence
{
  robot_msgs__msg__ChassisCmd * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} robot_msgs__msg__ChassisCmd__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // ROBOT_MSGS__MSG__DETAIL__CHASSIS_CMD__STRUCT_H_
