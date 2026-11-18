cmake_minimum_required(VERSION 3.8)
project(app_remote_video)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(robot_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(app_remote_video_node
  remote_video_node.cpp
)
ament_target_dependencies(app_remote_video_node rclcpp robot_msgs sensor_msgs std_msgs)

# 由 comm_webrtc 并入：WebRTC 媒体推流 + 信令
add_executable(comm_webrtc
  webrtc_node.cpp
)
ament_target_dependencies(comm_webrtc rclcpp sensor_msgs std_msgs)

install(TARGETS app_remote_video_node comm_webrtc
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()