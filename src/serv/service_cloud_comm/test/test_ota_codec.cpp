// =============================================================================
// test_ota_codec.cpp — L3 通信层 OTA 契约转换单测
// 覆盖协议 v1.3 §2.2.9（APP）、§2.2.10（CHASSIS）下行 JSON 与
//      §2.2.16 OTA 状态上行 JSON 的字段口径。
// =============================================================================
#include "service_cloud_comm/core/ota_codec.hpp"

#include <gtest/gtest.h>

using nlohmann::json;
using service_cloud_comm::core::decode_ota_command;
using service_cloud_comm::core::encode_ota_status;
using robot_msgs::msg::OtaCommand;
using robot_msgs::msg::OtaStatus;

namespace {

json app_payload() {
  return json::parse(R"({
    "msg_id": "uuid-o1",
    "timestamp": 1794535200000,
    "ota_task_id": "OTA-APP-001",
    "target_module": "APP",
    "target_version": "v1.3.0",
    "image_url": "https://ota.example.com/app_v1.3.0.tar",
    "sha256": "a1b2c3d4e5f6",
    "signature": "MEUCIQD",
    "delivery": "tar",
    "min_battery_soc": 30
  })");
}

json chassis_payload() {
  return json::parse(R"({
    "msg_id": "uuid-o2",
    "timestamp": 1794535200000,
    "ota_task_id": "OTA-CHASSIS-001",
    "target_module": "CHASSIS",
    "target_model": "STM32G474",
    "target_version": "v1.1.5",
    "firmware_url": "https://ota.example.com/stm32_fw_v1.1.5.bin",
    "sha256": "e5f6g7h8i9j0",
    "signature": "MEQCIF"
  })");
}

}  // namespace

// ---------------------------------------------------------------------------
// 下行：APP
// ---------------------------------------------------------------------------
TEST(OtaDecode, AppTar) {
  OtaCommand cmd;
  std::string err;
  ASSERT_TRUE(decode_ota_command(app_payload(), "cart-0000001", cmd, err)) << err;

  EXPECT_EQ(cmd.header.msg_id, "uuid-o1");
  EXPECT_EQ(cmd.header.timestamp_ms, 1794535200000LL);
  EXPECT_EQ(cmd.header.protocol_version, "1.3");
  EXPECT_EQ(cmd.header.cart_id, "cart-0000001");
  EXPECT_EQ(cmd.ota_task_id, "OTA-APP-001");
  EXPECT_EQ(cmd.target_module, OtaCommand::TARGET_APP);
  EXPECT_EQ(cmd.target_version, "v1.3.0");
  // 协议字段名是 image_url，ROS 契约统一为 url
  EXPECT_EQ(cmd.url, "https://ota.example.com/app_v1.3.0.tar");
  EXPECT_EQ(cmd.sha256, "a1b2c3d4e5f6");
  EXPECT_EQ(cmd.signature, "MEUCIQD");
  EXPECT_EQ(cmd.delivery, OtaCommand::DELIVERY_TAR);
  EXPECT_EQ(cmd.min_battery_soc, 30);
}

TEST(OtaDecode, AppPull) {
  json j = app_payload();
  j["delivery"] = "pull";
  OtaCommand cmd;
  std::string err;
  ASSERT_TRUE(decode_ota_command(j, "cart-1", cmd, err)) << err;
  EXPECT_EQ(cmd.delivery, OtaCommand::DELIVERY_PULL);
}

TEST(OtaDecode, AppMissingSha256Rejected) {
  json j = app_payload();
  j.erase("sha256");
  OtaCommand cmd;
  std::string err;
  EXPECT_FALSE(decode_ota_command(j, "cart-1", cmd, err));
  EXPECT_NE(err.find("sha256"), std::string::npos);
}

TEST(OtaDecode, AppInvalidDeliveryRejected) {
  json j = app_payload();
  j["delivery"] = "oci";
  OtaCommand cmd;
  std::string err;
  EXPECT_FALSE(decode_ota_command(j, "cart-1", cmd, err));
}

TEST(OtaDecode, AppSocOutOfRangeRejected) {
  json j = app_payload();
  j["min_battery_soc"] = 101;
  OtaCommand cmd;
  std::string err;
  EXPECT_FALSE(decode_ota_command(j, "cart-1", cmd, err));
}

// ---------------------------------------------------------------------------
// 下行：CHASSIS
// ---------------------------------------------------------------------------
TEST(OtaDecode, Chassis) {
  OtaCommand cmd;
  std::string err;
  ASSERT_TRUE(decode_ota_command(chassis_payload(), "cart-0000001", cmd, err)) << err;

  EXPECT_EQ(cmd.target_module, OtaCommand::TARGET_CHASSIS);
  EXPECT_EQ(cmd.target_model, "STM32G474");
  // firmware_url -> url
  EXPECT_EQ(cmd.url, "https://ota.example.com/stm32_fw_v1.1.5.bin");
  // 协议未给 delivery/min_battery_soc，按无镜像分发处理
  EXPECT_EQ(cmd.delivery, OtaCommand::DELIVERY_TAR);
  EXPECT_EQ(cmd.min_battery_soc, 0);
}

TEST(OtaDecode, ChassisMissingModelRejected) {
  json j = chassis_payload();
  j.erase("target_model");
  OtaCommand cmd;
  std::string err;
  EXPECT_FALSE(decode_ota_command(j, "cart-1", cmd, err));
}

TEST(OtaDecode, InvalidTargetModuleRejected) {
  json j = app_payload();
  j["target_module"] = "SYSTEM";
  OtaCommand cmd;
  std::string err;
  EXPECT_FALSE(decode_ota_command(j, "cart-1", cmd, err));
}

TEST(OtaDecode, NonObjectRejected) {
  OtaCommand cmd;
  std::string err;
  EXPECT_FALSE(decode_ota_command(json::array(), "cart-1", cmd, err));
}

// ---------------------------------------------------------------------------
// 上行：OtaStatus -> JSON
// ---------------------------------------------------------------------------
TEST(OtaEncode, NonTerminalResultIsNull) {
  OtaStatus s;
  s.header.msg_id = "uuid-9999";
  s.header.timestamp_ms = 1794535200000LL;
  s.ota_task_id = "OTA-APP-001";
  s.target_module = OtaStatus::TARGET_APP;
  s.target_version = "v1.3.0";
  s.stage = OtaStatus::STAGE_DOWNLOADING;
  s.progress = 45;
  s.result = OtaStatus::RESULT_NONE;

  json j = encode_ota_status(s);
  EXPECT_EQ(j["msg_id"], "uuid-9999");
  EXPECT_EQ(j["timestamp"], 1794535200000LL);
  EXPECT_EQ(j["ota_task_id"], "OTA-APP-001");
  EXPECT_EQ(j["target_module"], "APP");
  EXPECT_EQ(j["stage"], "DOWNLOADING");
  EXPECT_EQ(j["progress"], 45);
  EXPECT_TRUE(j["result"].is_null());
}

TEST(OtaEncode, FinishedSuccess) {
  OtaStatus s;
  s.ota_task_id = "OTA-CHASSIS-001";
  s.target_module = OtaStatus::TARGET_CHASSIS;
  s.stage = OtaStatus::STAGE_FINISHED;
  s.progress = 100;
  s.result = OtaStatus::RESULT_SUCCESS;

  json j = encode_ota_status(s);
  EXPECT_EQ(j["target_module"], "CHASSIS");
  EXPECT_EQ(j["stage"], "FINISHED");
  EXPECT_EQ(j["result"], "SUCCESS");
}

TEST(OtaEncode, RollbackFailed) {
  OtaStatus s;
  s.stage = OtaStatus::STAGE_ROLLBACK;
  s.result = OtaStatus::RESULT_ROLLBACK_FAILED;
  json j = encode_ota_status(s);
  EXPECT_EQ(j["stage"], "ROLLBACK");
  EXPECT_EQ(j["result"], "ROLLBACK_FAILED");
}
