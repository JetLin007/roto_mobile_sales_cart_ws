#include "service_cloud_comm/core/ota_codec.hpp"

#include <cstdint>

namespace service_cloud_comm {
namespace core {

namespace {

using json = nlohmann::json;

// 协议公共字段 protocol_version 固定值（同主版本兼容）
constexpr const char * kProtocolVersion = "1.3";

bool get_string(const json & j, const char * key, std::string & out,
                std::string & err) {
  auto it = j.find(key);
  if (it == j.end() || it->is_null()) {
    err = std::string("missing field: ") + key;
    return false;
  }
  if (!it->is_string()) {
    err = std::string("field not string: ") + key;
    return false;
  }
  out = it->get<std::string>();
  if (out.empty()) {
    err = std::string("empty field: ") + key;
    return false;
  }
  return true;
}

bool get_int(const json & j, const char * key, int64_t & out, std::string & err) {
  auto it = j.find(key);
  if (it == j.end() || it->is_null()) {
    err = std::string("missing field: ") + key;
    return false;
  }
  if (!it->is_number_integer()) {
    err = std::string("field not integer: ") + key;
    return false;
  }
  out = it->get<int64_t>();
  return true;
}

const char * target_module_to_string(uint8_t module) {
  return module == robot_msgs::msg::OtaStatus::TARGET_CHASSIS ? "CHASSIS" : "APP";
}

const char * stage_to_string(uint8_t stage) {
  using S = robot_msgs::msg::OtaStatus;
  switch (stage) {
    case S::STAGE_DOWNLOADING: return "DOWNLOADING";
    case S::STAGE_VERIFYING:   return "VERIFYING";
    case S::STAGE_INSTALLING:  return "INSTALLING";
    case S::STAGE_ROLLBACK:    return "ROLLBACK";
    case S::STAGE_FINISHED:    return "FINISHED";
    default:                   return "FINISHED";
  }
}

}  // namespace

bool decode_ota_command(const json & j, const std::string & cart_id,
                        robot_msgs::msg::OtaCommand & out, std::string & err) {
  using Cmd = robot_msgs::msg::OtaCommand;
  if (!j.is_object()) {
    err = "payload not a json object";
    return false;
  }

  std::string msg_id;
  int64_t timestamp_ms = 0;
  if (!get_string(j, "msg_id", msg_id, err)) { return false; }
  if (!get_int(j, "timestamp", timestamp_ms, err)) { return false; }
  if (!get_string(j, "ota_task_id", out.ota_task_id, err)) { return false; }
  if (!get_string(j, "target_version", out.target_version, err)) { return false; }
  if (!get_string(j, "sha256", out.sha256, err)) { return false; }
  if (!get_string(j, "signature", out.signature, err)) { return false; }

  out.header.msg_id = msg_id;
  out.header.timestamp_ms = timestamp_ms;
  out.header.protocol_version = kProtocolVersion;
  out.header.cart_id = cart_id;

  std::string module;
  auto it = j.find("target_module");
  if (it == j.end() || !it->is_string()) {
    err = "missing field: target_module";
    return false;
  }
  module = it->get<std::string>();
  if (module == "APP") {
    out.target_module = Cmd::TARGET_APP;
  } else if (module == "CHASSIS") {
    out.target_module = Cmd::TARGET_CHASSIS;
  } else {
    err = "invalid target_module: " + module;
    return false;
  }

  if (out.target_module == Cmd::TARGET_APP) {
    // §2.2.9：image_url / delivery / min_battery_soc 均必填
    if (!get_string(j, "image_url", out.url, err)) { return false; }
    std::string delivery;
    if (!get_string(j, "delivery", delivery, err)) { return false; }
    if (delivery == "tar") {
      out.delivery = Cmd::DELIVERY_TAR;
    } else if (delivery == "pull") {
      out.delivery = Cmd::DELIVERY_PULL;
    } else {
      err = "invalid delivery: " + delivery;
      return false;
    }
    int64_t soc = 0;
    if (!get_int(j, "min_battery_soc", soc, err)) { return false; }
    if (soc < 0 || soc > 100) {
      err = "min_battery_soc out of range";
      return false;
    }
    out.min_battery_soc = static_cast<uint8_t>(soc);
    out.target_model.clear();
  } else {
    // §2.2.10：target_model / firmware_url 必填，无 delivery/min_battery_soc
    if (!get_string(j, "target_model", out.target_model, err)) { return false; }
    if (!get_string(j, "firmware_url", out.url, err)) { return false; }
    out.delivery = Cmd::DELIVERY_TAR;
    out.min_battery_soc = 0;
  }
  return true;
}

json encode_ota_status(const robot_msgs::msg::OtaStatus & s) {
  using S = robot_msgs::msg::OtaStatus;
  json j;
  j["msg_id"] = s.header.msg_id;
  j["timestamp"] = s.header.timestamp_ms;
  j["ota_task_id"] = s.ota_task_id;
  j["target_module"] = target_module_to_string(s.target_module);
  j["target_version"] = s.target_version;
  j["stage"] = stage_to_string(s.stage);
  j["progress"] = static_cast<int>(s.progress);
  switch (s.result) {
    case S::RESULT_SUCCESS:          j["result"] = "SUCCESS"; break;
    case S::RESULT_FAILED:           j["result"] = "FAILED"; break;
    case S::RESULT_ROLLBACK_SUCCESS: j["result"] = "ROLLBACK_SUCCESS"; break;
    case S::RESULT_ROLLBACK_FAILED:  j["result"] = "ROLLBACK_FAILED"; break;
    default:                         j["result"] = nullptr; break;
  }
  return j;
}

}  // namespace core
}  // namespace service_cloud_comm
