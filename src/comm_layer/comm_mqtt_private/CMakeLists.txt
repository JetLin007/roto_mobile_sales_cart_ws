cmake_minimum_required(VERSION 3.8)
project(comm_mqtt_private)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(robot_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MQTT REQUIRED libpaho-mqttpp3)
pkg_check_modules(PAHO_C REQUIRED libpaho-mqtt3as)

include_directories(include ${MQTT_INCLUDE_DIRS})
link_directories(${MQTT_LIBRARY_DIRS})

add_executable(comm_mqtt_private
  mqtt_private_bridge.cpp
)
ament_target_dependencies(comm_mqtt_private rclcpp robot_msgs std_msgs)
target_link_libraries(comm_mqtt_private ${MQTT_LIBRARIES} ${PAHO_C_LIBRARIES})

install(TARGETS comm_mqtt_private
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()