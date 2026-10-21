cmake_minimum_required(VERSION 3.8)
project(driver_can_chassis)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(robot_msgs REQUIRED)
find_package(std_msgs REQUIRED)

include_directories(include)

add_executable(driver_can_chassis_stub
  src/chassis_driver_base.cpp
  src/chassis_stub_impl.cpp
)
ament_target_dependencies(driver_can_chassis_stub rclcpp robot_msgs std_msgs)

add_executable(driver_can_chassis_hw
  src/chassis_driver_base.cpp
  src/chassis_hw_impl.cpp
)
ament_target_dependencies(driver_can_chassis_hw rclcpp robot_msgs std_msgs)

install(TARGETS driver_can_chassis_stub driver_can_chassis_hw
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()