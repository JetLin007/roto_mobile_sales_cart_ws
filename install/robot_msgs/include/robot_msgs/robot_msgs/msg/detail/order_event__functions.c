// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from robot_msgs:msg/OrderEvent.idl
// generated code does not contain a copyright notice
#include "robot_msgs/msg/detail/order_event__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `header`
#include "std_msgs/msg/detail/header__functions.h"
// Member `order_id`
// Member `tablet_id`
// Member `payload_json`
#include "rosidl_runtime_c/string_functions.h"

bool
robot_msgs__msg__OrderEvent__init(robot_msgs__msg__OrderEvent * msg)
{
  if (!msg) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__init(&msg->header)) {
    robot_msgs__msg__OrderEvent__fini(msg);
    return false;
  }
  // order_id
  if (!rosidl_runtime_c__String__init(&msg->order_id)) {
    robot_msgs__msg__OrderEvent__fini(msg);
    return false;
  }
  // tablet_id
  if (!rosidl_runtime_c__String__init(&msg->tablet_id)) {
    robot_msgs__msg__OrderEvent__fini(msg);
    return false;
  }
  // type
  // target_lat
  // target_lon
  // payload_json
  if (!rosidl_runtime_c__String__init(&msg->payload_json)) {
    robot_msgs__msg__OrderEvent__fini(msg);
    return false;
  }
  return true;
}

void
robot_msgs__msg__OrderEvent__fini(robot_msgs__msg__OrderEvent * msg)
{
  if (!msg) {
    return;
  }
  // header
  std_msgs__msg__Header__fini(&msg->header);
  // order_id
  rosidl_runtime_c__String__fini(&msg->order_id);
  // tablet_id
  rosidl_runtime_c__String__fini(&msg->tablet_id);
  // type
  // target_lat
  // target_lon
  // payload_json
  rosidl_runtime_c__String__fini(&msg->payload_json);
}

bool
robot_msgs__msg__OrderEvent__are_equal(const robot_msgs__msg__OrderEvent * lhs, const robot_msgs__msg__OrderEvent * rhs)
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
  // order_id
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->order_id), &(rhs->order_id)))
  {
    return false;
  }
  // tablet_id
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->tablet_id), &(rhs->tablet_id)))
  {
    return false;
  }
  // type
  if (lhs->type != rhs->type) {
    return false;
  }
  // target_lat
  if (lhs->target_lat != rhs->target_lat) {
    return false;
  }
  // target_lon
  if (lhs->target_lon != rhs->target_lon) {
    return false;
  }
  // payload_json
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->payload_json), &(rhs->payload_json)))
  {
    return false;
  }
  return true;
}

bool
robot_msgs__msg__OrderEvent__copy(
  const robot_msgs__msg__OrderEvent * input,
  robot_msgs__msg__OrderEvent * output)
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
  // order_id
  if (!rosidl_runtime_c__String__copy(
      &(input->order_id), &(output->order_id)))
  {
    return false;
  }
  // tablet_id
  if (!rosidl_runtime_c__String__copy(
      &(input->tablet_id), &(output->tablet_id)))
  {
    return false;
  }
  // type
  output->type = input->type;
  // target_lat
  output->target_lat = input->target_lat;
  // target_lon
  output->target_lon = input->target_lon;
  // payload_json
  if (!rosidl_runtime_c__String__copy(
      &(input->payload_json), &(output->payload_json)))
  {
    return false;
  }
  return true;
}

robot_msgs__msg__OrderEvent *
robot_msgs__msg__OrderEvent__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  robot_msgs__msg__OrderEvent * msg = (robot_msgs__msg__OrderEvent *)allocator.allocate(sizeof(robot_msgs__msg__OrderEvent), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(robot_msgs__msg__OrderEvent));
  bool success = robot_msgs__msg__OrderEvent__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
robot_msgs__msg__OrderEvent__destroy(robot_msgs__msg__OrderEvent * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    robot_msgs__msg__OrderEvent__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
robot_msgs__msg__OrderEvent__Sequence__init(robot_msgs__msg__OrderEvent__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  robot_msgs__msg__OrderEvent * data = NULL;

  if (size) {
    data = (robot_msgs__msg__OrderEvent *)allocator.zero_allocate(size, sizeof(robot_msgs__msg__OrderEvent), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = robot_msgs__msg__OrderEvent__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        robot_msgs__msg__OrderEvent__fini(&data[i - 1]);
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
robot_msgs__msg__OrderEvent__Sequence__fini(robot_msgs__msg__OrderEvent__Sequence * array)
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
      robot_msgs__msg__OrderEvent__fini(&array->data[i]);
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

robot_msgs__msg__OrderEvent__Sequence *
robot_msgs__msg__OrderEvent__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  robot_msgs__msg__OrderEvent__Sequence * array = (robot_msgs__msg__OrderEvent__Sequence *)allocator.allocate(sizeof(robot_msgs__msg__OrderEvent__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = robot_msgs__msg__OrderEvent__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
robot_msgs__msg__OrderEvent__Sequence__destroy(robot_msgs__msg__OrderEvent__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    robot_msgs__msg__OrderEvent__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
robot_msgs__msg__OrderEvent__Sequence__are_equal(const robot_msgs__msg__OrderEvent__Sequence * lhs, const robot_msgs__msg__OrderEvent__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!robot_msgs__msg__OrderEvent__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
robot_msgs__msg__OrderEvent__Sequence__copy(
  const robot_msgs__msg__OrderEvent__Sequence * input,
  robot_msgs__msg__OrderEvent__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(robot_msgs__msg__OrderEvent);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    robot_msgs__msg__OrderEvent * data =
      (robot_msgs__msg__OrderEvent *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!robot_msgs__msg__OrderEvent__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          robot_msgs__msg__OrderEvent__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!robot_msgs__msg__OrderEvent__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
