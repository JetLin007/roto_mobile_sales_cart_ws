// NOLINT: This file starts with a BOM since it contain non-ASCII characters
// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from robot_msgs:msg/ChassisState.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__CHASSIS_STATE__STRUCT_H_
#define ROBOT_MSGS__MSG__DETAIL__CHASSIS_STATE__STRUCT_H_

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

/// Struct defined in msg/ChassisState in the package robot_msgs.
/**
  * 底盘反馈
 */
typedef struct robot_msgs__msg__ChassisState
{
  std_msgs__msg__Header header;
  float odom_x;
  float odom_y;
  float odom_theta;
  float vel_linear;
  float vel_angular;
  /// 0=IDLE 1=RUNNING 2=ERROR
  uint8_t state;
  /// cm
  uint8_t ultrasonic_front;
  /// cm
  uint8_t ultrasonic_rear;
} robot_msgs__msg__ChassisState;

// Struct for a sequence of robot_msgs__msg__ChassisState.
typedef struct robot_msgs__msg__ChassisState__Sequence
{
  robot_msgs__msg__ChassisState * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} robot_msgs__msg__ChassisState__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // ROBOT_MSGS__MSG__DETAIL__CHASSIS_STATE__STRUCT_H_
