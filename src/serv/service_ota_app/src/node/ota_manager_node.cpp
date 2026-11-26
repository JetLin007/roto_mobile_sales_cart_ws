// =============================================================================
// ota_manager_node.cpp — service_ota_app 的 ROS 壳层
// -----------------------------------------------------------------------------
// 职责（遵循 D6：node 层只做消息<->结构体转换 + 线程编排，不写业务判断）：
//   1. 订阅 /ota/command (OtaCommand) -> 转 OtaRequest -> 线程调 OtaCore::run
//   2. OtaCore 进度回调 -> 转 OtaStatus -> 发布 /ota/status
//   3. 节点启动时读 result.json，上报上次 OTA 最终结果（容器重启后补报）
// 业务逻辑（下载/校验/状态机）全部在 src/core/ota_core.cpp，可脱离 ROS 单测。
// =============================================================================
#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/ota_command.hpp"
#include "robot_msgs/msg/ota_status.hpp"

#include "service_ota_app/core/ota_core.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

using service_ota_app::core::Delivery;
using service_ota_app::core::OtaCore;
using service_ota_app::core::OtaProgress;
using service_ota_app::core::OtaRequest;
using service_ota_app::core::Result;
using service_ota_app::core::Stage;
using service_ota_app::core::TargetModule;

class OtaManagerNode : public rclcpp::Node {
public:
  OtaManagerNode() : Node("app_ota_manager") {
    // ---- 参数 ----
    this->declare_parameter("work_dir", "/opt/roto_ws/data/ota");
    this->declare_parameter("download_dir", "/opt/roto_ws/data/ota/cache");
    this->declare_parameter("ed25519_pubkey_pem", "");  // 空=跳过签名校验（仅调试）
    this->declare_parameter("cart_id", "cart-0000001");

    OtaCore::Options opts;
    opts.work_dir            = this->get_parameter("work_dir").as_string();
    opts.download_dir        = this->get_parameter("download_dir").as_string();
    opts.ed25519_pubkey_pem  = this->get_parameter("ed25519_pubkey_pem").as_string();
    cart_id_                 = this->get_parameter("cart_id").as_string();

    core_ = std::make_unique<OtaCore>(std::move(opts));
    // core 进度回调 -> 发布 OtaStatus
    core_->on_progress = [this](const OtaProgress & p) { this->publish_status(p); };

    // ---- ROS 接口 ----
    pub_status_ = this->create_publisher<robot_msgs::msg::OtaStatus>("/ota/status", 10);
    sub_cmd_    = this->create_subscription<robot_msgs::msg::OtaCommand>(
        "/ota/command", 10,
        std::bind(&OtaManagerNode::command_callback, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "app_ota_manager started, work_dir=%s",
                core_ ? "ok" : "fail");

    // 启动时尝试读取上次 OTA 结果（容器重启后补报最终状态）
    report_last_result();
  }

  ~OtaManagerNode() {
    running_ = false;
    if (worker_ && worker_->joinable()) worker_->join();
  }

private:
  // OtaCommand -> OtaRequest（剥离 ProtocolHeader，core 不关心消息头）
  static OtaRequest to_request(const robot_msgs::msg::OtaCommand & c) {
    OtaRequest r;
    r.ota_task_id    = c.ota_task_id;
    r.target_module  = static_cast<TargetModule>(c.target_module);
    r.target_model   = c.target_model;
    r.target_version = c.target_version;
    r.url            = c.url;
    r.sha256         = c.sha256;
    r.signature      = c.signature;
    r.delivery       = static_cast<Delivery>(c.delivery);
    r.min_battery_soc = c.min_battery_soc;
    return r;
  }

  // OtaProgress -> OtaStatus（node 层负责填充 ProtocolHeader）
  void publish_status(const OtaProgress & p) {
    auto msg = robot_msgs::msg::OtaStatus();
    msg.header.msg_id          = make_msg_id();
    msg.header.timestamp_ms    = now_ms();
    msg.header.protocol_version = "1.3";
    msg.header.cart_id         = cart_id_;
    msg.ota_task_id            = p.ota_task_id;
    msg.target_module          = static_cast<uint8_t>(p.target_module);
    msg.target_version         = p.target_version;
    msg.stage                  = static_cast<uint8_t>(p.stage);
    msg.progress               = p.progress;
    msg.result                 = static_cast<uint8_t>(p.result);
    pub_status_->publish(msg);
    RCLCPP_INFO(this->get_logger(), "OTA %s stage=%u progress=%u result=%u %s",
                p.ota_task_id.c_str(), msg.stage, msg.progress, msg.result,
                p.message.c_str());
  }

  // 订阅到 OtaCommand 后，丢到工作线程执行（run() 阻塞下载）
  void command_callback(const robot_msgs::msg::OtaCommand::SharedPtr c) {
    if (running_) {
      RCLCPP_WARN(this->get_logger(), "OTA task %s ignored: another task running",
                  c->ota_task_id.c_str());
      return;
    }
    OtaRequest req = to_request(*c);
    running_ = true;
    worker_ = std::make_unique<std::thread>([this, req]() {
      core_->run(req);
      running_ = false;
    });
  }

  // 启动时读取宿主 agent 写回的 result.json，上报上次 OTA 终态
  void report_last_result() {
    OtaProgress p;
    std::string err;
    if (core_->read_result(p, err)) {
      RCLCPP_INFO(this->get_logger(), "report last OTA result: %s", p.message.c_str());
      publish_status(p);
    }
  }

  static int64_t now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
  }
  static std::string make_msg_id() {
    return "ota-" + std::to_string(now_ms());
  }

  std::unique_ptr<OtaCore> core_;
  std::unique_ptr<std::thread> worker_;
  std::atomic<bool> running_{false};
  std::string cart_id_;
  rclcpp::Publisher<robot_msgs::msg::OtaStatus>::SharedPtr pub_status_;
  rclcpp::Subscription<robot_msgs::msg::OtaCommand>::SharedPtr sub_cmd_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<OtaManagerNode>());
  rclcpp::shutdown();
  return 0;
}
