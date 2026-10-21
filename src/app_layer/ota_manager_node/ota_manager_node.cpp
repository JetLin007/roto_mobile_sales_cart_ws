#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

/**
 * @brief OTA 升级管理节点
 * 
 * 接收云端升级指令 → 下载固件 → 校验 → 安装 → 重启
 */
class OtaManagerNode : public rclcpp::Node {
public:
  OtaManagerNode() : Node("app_ota_manager") {
    this->declare_parameter("ota_download_dir", "/tmp/ota");
    this->declare_parameter("current_version", "0.1.0");

    sub_cmd_ = this->create_subscription<std_msgs::msg::String>(
      "/ota/command", 10,
      std::bind(&OtaManagerNode::cmd_callback, this, std::placeholders::_1));

    pub_status_ = this->create_publisher<std_msgs::msg::String>("/ota/status", 10);

    RCLCPP_INFO(this->get_logger(), "app_ota_manager started, version: %s",
      get_parameter("current_version").as_string().c_str());
  }

private:
  void cmd_callback(const std_msgs::msg::String::SharedPtr msg) {
    // TODO: 解析 JSON 升级指令 {"action":"upgrade","url":"...","version":"0.2.0","md5":"..."}
    RCLCPP_INFO(this->get_logger(), "OTA command received: %s", msg->data.c_str());
    publish_status("idle", "Command received");
  }

  void publish_status(const std::string & state, const std::string & msg) {
    std_msgs::msg::String s;
    s.data = "{\"state\":\"" + state + "\",\"message\":\"" + msg + "\"}";
    pub_status_->publish(s);
  }

  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_cmd_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_status_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<OtaManagerNode>());
  rclcpp::shutdown();
  return 0;
}