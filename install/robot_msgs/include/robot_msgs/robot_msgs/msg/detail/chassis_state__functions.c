// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from robot_msgs:msg/ChassisState.idl
// generated code does not contain a copyright notice
#include "robot_msgs/msg/detail/chassis_state__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `header`
#include "std_msgs/msg/detail/header__functions.h"

bool
robot_msgs__msg__ChassisState__init(robot_msgs__msg__ChassisState * msg)
{
  if (!msg) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__init(&msg->header)) {
    robot_msgs__msg__ChassisState__fini(msg);
    return false;
  }
  // odom_x
  // odom_y
  // odom_theta
  // vel_linear
  // vel_angular
  // state
  // ultrasonic_front
  // ultrasonic_rear
  return true;
}

void
robot_msgs__msg__ChassisState__fini(robot_msgs__msg__ChassisState * msg)
{
  if (!msg) {
    return;
  }
  // header
  std_msgs__msg__Header__fini(&msg->header);
  // odom_x
  // odom_y
  // odom_theta
  // vel_linear
  // vel_angular
  // state
  // ultrasonic_front
  // ultrasonic_rear
}

bool
robot_msgs__msg__ChassisState__are_equal(const robot_msgs__msg__ChassisState * lhs, const robot_msgs__msg__ChassisState * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__are_equal(
      &(lhs->header), &(rhs->header)))
  {
    return false;
  }
  // odom_x
  if (lhs->odom_x != rhs->odom_x) {
    return false;
  }
  // odom_y
  if (lhs->odom_y != rhs->odom_y) {
    return false;
  }
  // odom_theta
  if (lhs->odom_theta != rhs->odom_theta) {
    return false;
  }
  // vel_linear
  if (lhs->vel_linear != rhs->vel_linear) {
    return false;
  }
  // vel_angular
  if (lhs->vel_angular != rhs->vel_angular) {
    return false;
  }
  // state
  if (lhs->state != rhs->state) {
    return false;
  }
  // ultrasonic_front
  if (lhs->ultrasonic_front != rhs->ultrasonic_front) {
    return false;
  }
  // ultrasonic_rear
  if (lhs->ultrasonic_rear != rhs->ultrasonic_rear) {
    return false;
  }
  return true;
}

bool
robot_msgs__msg__ChassisState__copy(
  const robot_msgs__msg__ChassisState * input,
  robot_msgs__msg__ChassisState * output)
{
  if (!input || !output) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__copy(
      &(input->header), &(output->header)))
  {
    return false;
  }
  // odom_x
  output->odom_x = input->odom_x;
  // odom_y
  output->odom_y = input->odom_y;
  // odom_theta
  output->odom_theta = input->odom_theta;
  // vel_linear
  output->vel_linear = input->vel_linear;
  // vel_angular
  output->vel_angular = input->vel_angular;
  // state
  output->state = input->state;
  // ultrasonic_front
  output->ultrasonic_front = input->ultrasonic_front;
  // ultrasonic_rear
  output->ultrasonic_rear = input->ultrasonic_rear;
  return true;
}

robot_msgs__msg__ChassisState *
robot_msgs__msg__ChassisState__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  robot_msgs__msg__ChassisState * msg = (robot_msgs__msg__ChassisState *)allocator.allocate(sizeof(robot_msgs__msg__ChassisState), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(robot_msgs__msg__ChassisState));
  bool success = robot_msgs__msg__ChassisState__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
robot_msgs__msg__ChassisState__destroy(robot_msgs__msg__ChassisState * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    robot_msgs__msg__ChassisState__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
robot_msgs__msg__ChassisState__Sequence__init(robot_msgs__msg__ChassisState__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  robot_msgs__msg__ChassisState * data = NULL;

  if (size) {
    data = (robot_msgs__msg__ChassisState *)allocator.zero_allocate(size, sizeof(robot_msgs__msg__ChassisState), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = robot_msgs__msg__ChassisState__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        robot_msgs__msg__ChassisState__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
robot_msgs__msg__ChassisState__Sequence__fini(robot_msgs__msg__ChassisState__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      robot_msgs__msg__ChassisState__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

robot_msgs__msg__ChassisState__Sequence *
robot_msgs__msg__ChassisState__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  robot_msgs__msg__ChassisState__Sequence * array = (robot_msgs__msg__ChassisState__Sequence *)allocator.allocate(sizeof(robot_msgs__msg__ChassisState__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = robot_msgs__msg__ChassisState__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
robot_msgs__msg__ChassisState__Sequence__destroy(robot_msgs__msg__ChassisState__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    robot_msgs__msg__ChassisState__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
robot_msgs__msg__ChassisState__Sequence__are_equal(const robot_msgs__msg__ChassisState__Sequence * lhs, const robot_msgs__msg__ChassisState__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!robot_msgs__msg__ChassisState__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
robot_msgs__msg__ChassisState__Sequence__copy(
  const robot_msgs__msg__ChassisState__Sequence * input,
  robot_msgs__msg__ChassisState__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(robot_msgs__msg__ChassisState);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    robot_msgs__msg__ChassisState * data =
      (robot_msgs__msg__ChassisState *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!robot_msgs__msg__ChassisState__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          robot_msgs__msg__ChassisState__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!robot_msgs__msg__ChassisState__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
