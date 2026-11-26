#ifndef SERVICE_CLOUD_COMM_CORE_OTA_CODEC_HPP_
#define SERVICE_CLOUD_COMM_CORE_OTA_CODEC_HPP_

#include <string>

#include <nlohmann/json.hpp>

#include "robot_msgs/msg/ota_command.hpp"
#include "robot_msgs/msg/ota_status.hpp"

namespace service_cloud_comm {
namespace core {

// =============================================================================
// OTA 协议编解码（协议 v1.3 §2.2.9 / §2.2.10 / §2.2.16）
// -----------------------------------------------------------------------------
// 纯 C++17 + robot_msgs（不含 rclcpp），可脱离 ROS 单测。
// 职责：把云端 JSON（字符串枚举、image_url/firmware_url）与 ROS 消息
//      （uint8 枚举、统一 url 字段）互转，是 L3 通信层唯一的字段口径入口。
// =============================================================================

// 下行：ota/app/version 或 ota/chassis/version 的 JSON -> OtaCommand
// cart_id 由 L3 节点注入（协议 payload 不含整车标识）。
// 返回 false 时 err 说明缺失/非法字段，调用方应丢弃该条指令并记 INTERNAL_ERROR。
bool decode_ota_command(const nlohmann::json & j, const std::string & cart_id,
                        robot_msgs::msg::OtaCommand & out, std::string & err);

// 上行：OtaStatus -> ota/status JSON。非终态 result 输出 null（协议要求）。
nlohmann::json encode_ota_status(const robot_msgs::msg::OtaStatus & s);

}  // namespace core
}  // namespace service_cloud_comm

#endif  // SERVICE_CLOUD_COMM_CORE_OTA_CODEC_HPP_
