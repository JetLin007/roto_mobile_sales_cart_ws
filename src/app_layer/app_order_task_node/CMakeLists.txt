cmake_minimum_required(VERSION 3.8)
project(app_order_task)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(robot_msgs REQUIRED)
find_package(std_msgs REQUIRED)

include_directories(include)

add_executable(app_order_task_node
  order_task_node.cpp
)
ament_target_dependencies(app_order_task_node rclcpp robot_msgs std_msgs)

install(TARGETS app_order_task_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()