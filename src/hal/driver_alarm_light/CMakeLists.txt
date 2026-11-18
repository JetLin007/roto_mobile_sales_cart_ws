cmake_minimum_required(VERSION 3.8)
project(driver_alarm_light)
# 分层归属：L2-HAL 交互
# 约束（docs/00-接口裁决.md D6）：src/core 纯 C++（不 include rclcpp）；src/node 只做 ROS 壳
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(robot_msgs REQUIRED)
ament_package()
