#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/chassis_cmd.hpp"
#include <mqtt/async_client.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @brief MQTT 回调类（独立出来避免多继承问题）
 */
class MqttCallbackHandler : public virtual mqtt::callback {
public:
  MqttCallbackHandler(std::function<void(mqtt::const_message_ptr)> msg_cb,
                      std::function<void(const std::string &)> conn_lost_cb)
  : msg_cb_(std::move(msg_cb)), conn_lost_cb_(std::move(conn_lost_cb)) {}

  void message_arrived(mqtt::const_message_ptr msg) override {
    msg_cb_(msg);
  }

  void connection_lost(const std::string & cause) override {
    conn_lost_cb_(cause);
  }

  void delivery_complete(mqtt::delivery_token_ptr) override {}

private:
  std::function<void(mqtt::const_message_ptr)> msg_cb_;
  std::function<void(const std::string &)> conn_lost_cb_;
};

class RemoteHandle : public rclcpp::Node {
public:
  RemoteHandle() : Node("comm_remote_handle") {
    this->declare_parameter("mqtt_broker", "127.0.0.1");
    this->declare_parameter("mqtt_port", 1883);
    this->declare_parameter("max_linear", 1.0f);
    this->declare_parameter("max_angular", 0.5f);

    std::string broker = get_parameter("mqtt_broker").as_string();
    int port = static_cast<int>(get_parameter("mqtt_port").as_int());
    max_linear_ = static_cast<float>(get_parameter("max_linear").as_double());
    max_angular_ = static_cast<float>(get_parameter("max_angular").as_double());

    std::string uri = "tcp://" + broker + ":" + std::to_string(port);
    mqtt_client_ = std::make_shared<mqtt::async_client>(uri, "remote_handle");

        // 设置回调
    callback_handler_ = std::make_unique<MqttCallbackHandler>(
      [this](mqtt::const_message_ptr msg) { this->on_message(msg); },
      [this](const std::string & cause) {
        RCLCPP_WARN(this->get_logger(), "MQTT lost: %s", cause.c_str());
      });
    mqtt_client_->set_callback(*callback_handler_);

    mqtt::connect_options opts;
    opts.set_keep_alive_interval(20);
    opts.set_clean_session(true);
    mqtt_client_->connect(opts)->wait();
    mqtt_client_->subscribe("remote/joystick", 1)->wait();

    pub_cmd_ = this->create_publisher<robot_msgs::msg::ChassisCmd>("/chassis/cmd", 10);

    RCLCPP_INFO(this->get_logger(), "comm_remote_handle started");
  }

private:
  void on_message(mqtt::const_message_ptr msg) {
    if (msg->get_topic() == "remote/joystick") {
      try {
        auto j = json::parse(msg->to_string());
        auto cmd = robot_msgs::msg::ChassisCmd();
        cmd.header.stamp = this->now();
        cmd.linear_x = j.value("x", 0.0f) * max_linear_;
        cmd.angular_z = j.value("z", 0.0f) * max_angular_;
        cmd.gear = (cmd.linear_x > 0) ? 1 : ((cmd.linear_x < 0) ? 2 : 0);
        pub_cmd_->publish(cmd);
      } catch (const std::exception & e) {
        RCLCPP_ERROR(this->get_logger(), "Parse joystick failed: %s", e.what());
      }
    }
  }

  std::shared_ptr<mqtt::async_client> mqtt_client_;
  std::unique_ptr<MqttCallbackHandler> callback_handler_;
  rclcpp::Publisher<robot_msgs::msg::ChassisCmd>::SharedPtr pub_cmd_;
  float max_linear_, max_angular_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<RemoteHandle>());
  rclcpp::shutdown();
  return 0;
}