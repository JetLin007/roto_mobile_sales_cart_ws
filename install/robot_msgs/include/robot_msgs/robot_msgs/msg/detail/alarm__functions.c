// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from robot_msgs:msg/Alarm.idl
// generated code does not contain a copyright notice
#include "robot_msgs/msg/detail/alarm__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `header`
#include "std_msgs/msg/detail/header__functions.h"
// Member `source`
// Member `message`
#include "rosidl_runtime_c/string_functions.h"

bool
robot_msgs__msg__Alarm__init(robot_msgs__msg__Alarm * msg)
{
  if (!msg) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__init(&msg->header)) {
    robot_msgs__msg__Alarm__fini(msg);
    return false;
  }
  // alarm_code
  // level
  // source
  if (!rosidl_runtime_c__String__init(&msg->source)) {
    robot_msgs__msg__Alarm__fini(msg);
    return false;
  }
  // message
  if (!rosidl_runtime_c__String__init(&msg->message)) {
    robot_msgs__msg__Alarm__fini(msg);
    return false;
  }
  // cleared
  return true;
}

void
robot_msgs__msg__Alarm__fini(robot_msgs__msg__Alarm * msg)
{
  if (!msg) {
    return;
  }
  // header
  std_msgs__msg__Header__fini(&msg->header);
  // alarm_code
  // level
  // source
  rosidl_runtime_c__String__fini(&msg->source);
  // message
  rosidl_runtime_c__String__fini(&msg->message);
  // cleared
}

bool
robot_msgs__msg__Alarm__are_equal(const robot_msgs__msg__Alarm * lhs, const robot_msgs__msg__Alarm * rhs)
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
  // alarm_code
  if (lhs->alarm_code != rhs->alarm_code) {
    return false;
  }
  // level
  if (lhs->level != rhs->level) {
    return false;
  }
  // source
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->source), &(rhs->source)))
  {
    return false;
  }
  // message
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->message), &(rhs->message)))
  {
    return false;
  }
  // cleared
  if (lhs->cleared != rhs->cleared) {
    return false;
  }
  return true;
}

bool
robot_msgs__msg__Alarm__copy(
  const robot_msgs__msg__Alarm * input,
  robot_msgs__msg__Alarm * output)
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
  // alarm_code
  output->alarm_code = input->alarm_code;
  // level
  output->level = input->level;
  // source
  if (!rosidl_runtime_c__String__copy(
      &(input->source), &(output->source)))
  {
    return false;
  }
  // message
  if (!rosidl_runtime_c__String__copy(
      &(input->message), &(output->message)))
  {
    return false;
  }
  // cleared
  output->cleared = input->cleared;
  return true;
}

robot_msgs__msg__Alarm *
robot_msgs__msg__Alarm__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  robot_msgs__msg__Alarm * msg = (robot_msgs__msg__Alarm *)allocator.allocate(sizeof(robot_msgs__msg__Alarm), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(robot_msgs__msg__Alarm));
  bool success = robot_msgs__msg__Alarm__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
robot_msgs__msg__Alarm__destroy(robot_msgs__msg__Alarm * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    robot_msgs__msg__Alarm__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
robot_msgs__msg__Alarm__Sequence__init(robot_msgs__msg__Alarm__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  robot_msgs__msg__Alarm * data = NULL;

  if (size) {
    data = (robot_msgs__msg__Alarm *)allocator.zero_allocate(size, sizeof(robot_msgs__msg__Alarm), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = robot_msgs__msg__Alarm__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        robot_msgs__msg__Alarm__fini(&data[i - 1]);
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
robot_msgs__msg__Alarm__Sequence__fini(robot_msgs__msg__Alarm__Sequence * array)
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
      robot_msgs__msg__Alarm__fini(&array->data[i]);
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

robot_msgs__msg__Alarm__Sequence *
robot_msgs__msg__Alarm__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  robot_msgs__msg__Alarm__Sequence * array = (robot_msgs__msg__Alarm__Sequence *)allocator.allocate(sizeof(robot_msgs__msg__Alarm__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = robot_msgs__msg__Alarm__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
robot_msgs__msg__Alarm__Sequence__destroy(robot_msgs__msg__Alarm__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    robot_msgs__msg__Alarm__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
robot_msgs__msg__Alarm__Sequence__are_equal(const robot_msgs__msg__Alarm__Sequence * lhs, const robot_msgs__msg__Alarm__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!robot_msgs__msg__Alarm__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
robot_msgs__msg__Alarm__Sequence__copy(
  const robot_msgs__msg__Alarm__Sequence * input,
  robot_msgs__msg__Alarm__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(robot_msgs__msg__Alarm);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    robot_msgs__msg__Alarm * data =
      (robot_msgs__msg__Alarm *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!robot_msgs__msg__Alarm__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          robot_msgs__msg__Alarm__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!robot_msgs__msg__Alarm__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
