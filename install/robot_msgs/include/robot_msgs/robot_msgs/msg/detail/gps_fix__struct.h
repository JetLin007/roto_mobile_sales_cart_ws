// NOLINT: This file starts with a BOM since it contain non-ASCII characters
// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from robot_msgs:msg/GpsFix.idl
// generated code does not contain a copyright notice

#ifndef ROBOT_MSGS__MSG__DETAIL__GPS_FIX__STRUCT_H_
#define ROBOT_MSGS__MSG__DETAIL__GPS_FIX__STRUCT_H_

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

/// Struct defined in msg/GpsFix in the package robot_msgs.
/**
  * GPS 定位数据
 */
typedef struct robot_msgs__msg__GpsFix
{
  std_msgs__msg__Header header;
  /// 纬度 (deg)
  double latitude;
  /// 经度 (deg)
  double longitude;
  /// 海拔 (m)
  float altitude;
  /// m/s
  float speed;
  /// 航向角 (deg, 0~360)
  float heading;
  /// 0=no fix, 1=GPS, 2=DGPS, 4=RTK, 5=RTK float
  uint8_t fix_type;
  uint8_t satellites;
  float hdop;
  float vdop;
} robot_msgs__msg__GpsFix;

// Struct for a sequence of robot_msgs__msg__GpsFix.
typedef struct robot_msgs__msg__GpsFix__Sequence
{
  robot_msgs__msg__GpsFix * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} robot_msgs__msg__GpsFix__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // ROBOT_MSGS__MSG__DETAIL__GPS_FIX__STRUCT_H_
