cmake_minimum_required(VERSION 3.8)
project(sim_tablet)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(robot_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)

add_executable(sim_tablet
  sim_tablet_node.cpp
)
ament_target_dependencies(sim_tablet rclcpp robot_msgs std_msgs std_srvs)

install(TARGETS sim_tablet
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()