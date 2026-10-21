cmake_minimum_required(VERSION 3.8)
project(driver_gps)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(robot_msgs REQUIRED)
find_package(std_msgs REQUIRED)

# ---- 公共头 ----
include_directories(include)

# ---- STUB 实现（仿真，无需硬件）----
add_executable(driver_gps_stub
  src/gps_driver_base.cpp
  src/gps_stub_impl.cpp
)
ament_target_dependencies(driver_gps_stub rclcpp robot_msgs std_msgs)

# ---- HARDWARE 实现（实车串口/GNSS）----
add_executable(driver_gps_hw
  src/gps_driver_base.cpp
  src/gps_hw_impl.cpp
)
ament_target_dependencies(driver_gps_hw rclcpp robot_msgs std_msgs)

install(TARGETS driver_gps_stub driver_gps_hw
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()