cmake_minimum_required(VERSION 3.8)
project(comm_webrtc)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)

include_directories(include)

add_executable(comm_webrtc
  src/webrtc_node.cpp
)
ament_target_dependencies(comm_webrtc rclcpp sensor_msgs std_msgs)

# TODO: 链接 WebRTC 库（如 libdatachannel 或 GStreamer）
# target_link_libraries(comm_webrtc datachannel)

install(TARGETS comm_webrtc
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()