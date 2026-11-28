cmake_minimum_required(VERSION 3.8)
project(service_cloud_comm)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(robot_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(PahoMqttCpp REQUIRED)

include_directories(include)

add_executable(comm_mqtt_cloud
  mqtt_cloud_bridge.cpp
  src/core/ota_codec.cpp      # OTA 协议 JSON <-> robot_msgs（纯 C++，无 rclcpp）
)
ament_target_dependencies(comm_mqtt_cloud rclcpp robot_msgs std_msgs nlohmann_json)
target_link_libraries(comm_mqtt_cloud PahoMqttCpp::paho-mqttpp3)

# 由 comm_remote_handle 并入：MQTT 遥杆 → ROS 底盘指令
add_executable(comm_remote_handle
  remote_handle.cpp
)
ament_target_dependencies(comm_remote_handle rclcpp robot_msgs std_msgs nlohmann_json)
target_link_libraries(comm_remote_handle PahoMqttCpp::paho-mqttpp3)

install(TARGETS comm_mqtt_cloud comm_remote_handle
  DESTINATION lib/${PROJECT_NAME}
)

# OTA 契约转换单测（L2 前置：先钉住 JSON <-> robot_msgs 的字段口径）
if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_ota_codec test/test_ota_codec.cpp src/core/ota_codec.cpp)
  ament_target_dependencies(test_ota_codec robot_msgs nlohmann_json)
endif()

ament_package()