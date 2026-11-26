cmake_minimum_required(VERSION 3.8)
project(service_ota_app)

# 分层归属：L3-Service
# 约束（docs/00-接口裁决.md D6）：src/core 纯 C++（不 include rclcpp）；src/node 只做 ROS 壳

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(robot_msgs REQUIRED)

# OTA 核心依赖：libcurl 下载、OpenSSL 哈希+签名、nlohmann/json 握手文件
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

include_directories(include)

add_executable(app_ota_manager_node
  src/core/crypto.cpp        # sha256 + Ed25519 校验
  src/core/ota_core.cpp      # 下载 + 状态机 + 文件握手
  src/node/ota_manager_node.cpp  # ROS 壳：消息转换 + 线程编排
)
ament_target_dependencies(app_ota_manager_node rclcpp robot_msgs)

target_link_libraries(app_ota_manager_node
  OpenSSL::SSL
  OpenSSL::Crypto
  CURL::libcurl
  nlohmann_json::nlohmann_json
)

install(TARGETS app_ota_manager_node
  DESTINATION lib/${PROJECT_NAME}
)

# 导出 core 头文件，供单测或其他包复用
install(DIRECTORY include/
  DESTINATION include
)

# core 层单测（L1 回归：sha256 / Ed25519 / 下载校验 / result 解析）
if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_ota_core
    test/test_ota_core.cpp
    src/core/crypto.cpp
    src/core/ota_core.cpp
  )
  target_link_libraries(test_ota_core
    OpenSSL::SSL
    OpenSSL::Crypto
    CURL::libcurl
    nlohmann_json::nlohmann_json
  )
endif()

ament_package()
