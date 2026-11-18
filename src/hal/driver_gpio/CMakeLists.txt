cmake_minimum_required(VERSION 3.8)
project(driver_gpio)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(robot_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)

include_directories(include)

add_executable(driver_gpio_stub
  src/gpio_driver_base.cpp
  src/gpio_stub_impl.cpp
)
ament_target_dependencies(driver_gpio_stub rclcpp robot_msgs std_msgs std_srvs)

add_executable(driver_gpio_hw
  src/gpio_driver_base.cpp
  src/gpio_hw_impl.cpp
)
ament_target_dependencies(driver_gpio_hw rclcpp robot_msgs std_msgs std_srvs)

install(TARGETS driver_gpio_stub driver_gpio_hw
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()