cmake_minimum_required(VERSION 3.8)
project(driver_camera)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)

find_path(RKMPP_INCLUDE_DIR rk_mpi.h PATHS /usr/include/rockchip /usr/local/include)
find_path(RKGA_INCLUDE_DIR drmrga.h PATHS /usr/include/rockchip /usr/local/include)

include_directories(include)

add_executable(driver_camera_stub
  src/camera_driver_base.cpp
  src/camera_stub_impl.cpp
)
ament_target_dependencies(driver_camera_stub rclcpp sensor_msgs std_msgs)

add_executable(driver_camera_hw
  src/camera_driver_base.cpp
  src/camera_hw_impl.cpp
)
ament_target_dependencies(driver_camera_hw rclcpp sensor_msgs std_msgs)

if(RKMPP_INCLUDE_DIR AND RKGA_INCLUDE_DIR)
  target_include_directories(driver_camera_hw PRIVATE ${RKMPP_INCLUDE_DIR} ${RKGA_INCLUDE_DIR})
  target_link_libraries(driver_camera_hw rockchip_mpp rockchip_rga)
  message(STATUS "RK MPP/RGA found, hardware encoding enabled")
else()
  message(WARNING "RK MPP/RGA not found, hardware encoding disabled")
endif()

install(TARGETS driver_camera_stub driver_camera_hw
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
