#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/cmd_vel.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include "std_msgs/msg/bool.hpp"

/**
 * @brief 远程视频 + 遥控业务节点
 *
 * 协调视频推流和远程手柄控制
 * 当远程连接时自动开始推流，断开时停止
 */
class RemoteVideoNode : public rclcpp::Node {
public:
  RemoteVideoNode() : Node("app_remote_video") {
    this->declare_parameter("max_remote_speed", 0.8f);

    sub_webrtc_status_ = this->create_subscription<std_msgs::msg::Bool>(
      "/webrtc/status", 10,
      std::bind(&RemoteVideoNode::webrtc_callback, this, std::placeholders::_1));

    sub_remote_cmd_ = this->create_subscription<robot_msgs::msg::CmdVel>(
      "/remote/cmd_vel", 10,
      std::bind(&RemoteVideoNode::remote_cmd_callback, this, std::placeholders::_1));

    pub_chassis_cmd_ = this->create_publisher<robot_msgs::msg::CmdVel>("/cmd_vel", 10);

    RCLCPP_INFO(this->get_logger(), "app_remote_video started");
  }

private:
  void webrtc_callback(const std_msgs::msg::Bool::SharedPtr msg) {
    if (msg->data) {
      RCLCPP_INFO(this->get_logger(), "Remote video connected, teleop enabled");
      remote_active_ = true;
    } else {
      RCLCPP_INFO(this->get_logger(), "Remote video disconnected, teleop disabled");
      remote_active_ = false;
      send_stop();
    }
  }

  void remote_cmd_callback(const robot_msgs::msg::CmdVel::SharedPtr msg) {
    if (!remote_active_) return;

    float max_speed = static_cast<float>(get_parameter("max_remote_speed").as_double());
    auto cmd = *msg;
    // 限速
    if (cmd.linear_x > max_speed) cmd.linear_x = max_speed;
    if (cmd.linear_x < -max_speed) cmd.linear_x = -max_speed;
    cmd.source = robot_msgs::msg::CmdVel::SRC_CLOUD;
    pub_chassis_cmd_->publish(cmd);
  }

  void send_stop() {
    auto cmd = robot_msgs::msg::CmdVel();
    cmd.header.timestamp_ms = static_cast<int64_t>(this->now().seconds() * 1000);
    cmd.header.protocol_version = "1.3";
    cmd.linear_x = 0;
    cmd.angular_z = 0;
    cmd.enable = false;
    cmd.source = robot_msgs::msg::CmdVel::SRC_CLOUD;
    pub_chassis_cmd_->publish(cmd);
  }

  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr sub_webrtc_status_;
  rclcpp::Subscription<robot_msgs::msg::CmdVel>::SharedPtr sub_remote_cmd_;
  rclcpp::Publisher<robot_msgs::msg::CmdVel>::SharedPtr pub_chassis_cmd_;
  bool remote_active_ = false;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<RemoteVideoNode>());
  rclcpp::shutdown();
  return 0;
}
