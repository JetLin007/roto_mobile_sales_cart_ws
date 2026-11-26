#ifndef SERVICE_OTA_APP_CORE_OTA_TYPES_HPP_
#define SERVICE_OTA_APP_CORE_OTA_TYPES_HPP_

#include <cstdint>
#include <string>

namespace service_ota_app {
namespace core {

// =============================================================================
// OTA 相关类型定义（纯数据结构，无 ROS 依赖）
// 对应 robot_msgs 中的 OtaCommand.msg / OtaStatus.msg，但解耦消息类型，
// 使 core 层可脱离 ROS 在 WSL2 上用 gtest 单测。
// =============================================================================

// OTA 目标模块：APP=容器镜像，CHASSIS=底盘 STM32 固件（YMODEM）
enum class TargetModule : uint8_t { kApp = 0, kChassis = 1 };

// 分发方式：TAR=下载离线 tar 包后 docker load；PULL=直接 docker pull registry
enum class Delivery : uint8_t { kTar = 0, kPull = 1 };

// OTA 执行阶段（对应 OtaStatus.stage）
enum class Stage : uint8_t {
  kDownloading = 0,  // 下载镜像/固件
  kVerifying = 1,    // sha256 + Ed25519 签名校验
  kInstalling = 2,   // 交给宿主 agent 安装（docker load / 切 tag / 重启）
  kRollback = 3,     // 回滚中
  kFinished = 4      // 终态（成功或失败）
};

// OTA 结果（对应 OtaStatus.result，非终态填 kNone）
enum class Result : uint8_t {
  kNone = 0,
  kSuccess = 1,
  kFailed = 2,
  kRollbackSuccess = 3,
  kRollbackFailed = 4
};

// OtaCommand 的核心字段（去掉 ProtocolHeader，core 层不关心消息头）
struct OtaRequest {
  std::string ota_task_id;          // 云端下发的任务 ID，幂等键
  TargetModule target_module = TargetModule::kApp;
  std::string target_model;         // CHASSIS 时如 STM32G474；APP 留空
  std::string target_version;       // 目标版本号
  std::string url;                  // 镜像/固件下载地址（TAR 模式）或 registry 引用（PULL 模式）
  std::string sha256;               // 完整性校验摘要（小写 hex）
  std::string signature;            // Ed25519 分离签名（base64），架构 E8
  Delivery delivery = Delivery::kTar;
  uint8_t min_battery_soc = 0;      // APP 升级要求的最低电量，避免升级中断电
};

// OTA 进度上报（对应 OtaStatus 的业务字段）
struct OtaProgress {
  std::string ota_task_id;
  TargetModule target_module = TargetModule::kApp;
  std::string target_version;
  Stage stage = Stage::kDownloading;
  uint8_t progress = 0;             // 0..100
  Result result = Result::kNone;    // 非终态为 kNone
  std::string message;              // 人类可读说明
};

// =============================================================================
// 容器 <-> 宿主 cart-ota-agent 的握手数据结构
// 序列化到 /opt/roto_ws/data/ota/request.json / result.json
// 容器无法 docker 操作，必须由宿主 agent 执行镜像替换与容器重启。
// =============================================================================

// 容器 -> 宿主 agent 的安装请求
struct HandoffRequest {
  std::string ota_task_id;
  std::string target_version;
  std::string action;               // install | rollback
  std::string delivery;             // tar | pull
  std::string image_ref;            // pull 模式：镜像引用（如 registry/roto-cart:1.3.0-arm64）
  std::string tar_path;             // tar 模式：镜像 tar 绝对路径（容器内路径，宿主需挂载同一卷）
  std::string sha256;               // 镜像摘要（agent 二次校验 docker load 后的镜像 ID/label）
  int64_t requested_at_ms = 0;      // UTC 毫秒
  int32_t min_battery_soc = 0;
};

// 宿主 agent -> 容器 的执行结果
struct HandoffResult {
  std::string ota_task_id;
  std::string status;               // installing | success | failed | rollback_success | rollback_failed
  std::string message;
  std::string image_ref;
  int64_t timestamp_ms = 0;
};

}  // namespace core
}  // namespace service_ota_app

#endif  // SERVICE_OTA_APP_CORE_OTA_TYPES_HPP_
