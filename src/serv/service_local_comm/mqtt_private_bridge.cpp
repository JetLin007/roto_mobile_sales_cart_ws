#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/task_event.hpp"
#include "robot_msgs/msg/chassis_status.hpp"
#include "robot_msgs/msg/battery_status.hpp"
#include <mqtt/async_client.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class MqttCallbackHandler : public virtual mqtt::callback {
public:
  MqttCallbackHandler(std::function<void(mqtt::const_message_ptr)> msg_cb,
                      std::function<void(const std::string &)> conn_lost_cb)
  : msg_cb_(std::move(msg_cb)), conn_lost_cb_(std::move(conn_lost_cb)) {}

  void message_arrived(mqtt::const_message_ptr msg) override { msg_cb_(msg); }
  void connection_lost(const std::string & cause) override { conn_lost_cb_(cause); }
  void delivery_complete(mqtt::delivery_token_ptr) override {}

private:
  std::function<void(mqtt::const_message_ptr)> msg_cb_;
  std::function<void(const std::string &)> conn_lost_cb_;
};

class MqttPrivateBridge : public rclcpp::Node {
public:
  MqttPrivateBridge() : Node("comm_mqtt_private") {
    this->declare_parameter("mqtt_broker", "127.0.0.1");
    this->declare_parameter("mqtt_port", 1883);
    this->declare_parameter("mqtt_client_id", "roto_cart_private");

    std::string broker = get_parameter("mqtt_broker").as_string();
    int port = static_cast<int>(get_parameter("mqtt_port").as_int());
    std::string client_id = get_parameter("mqtt_client_id").as_string();

    mqtt_client_ = std::make_shared<mqtt::async_client>(
      "tcp://" + broker + ":" + std::to_string(port), client_id);

    callback_handler_ = std::make_unique<MqttCallbackHandler>(
      [this](mqtt::const_message_ptr msg) { this->on_message(msg); },
      [this](const std::string & cause) {
        RCLCPP_WARN(this->get_logger(), "MQTT lost: %s", cause.c_str());
      });
    mqtt_client_->set_callback(*callback_handler_);

    mqtt::connect_options conn_opts;
    conn_opts.set_keep_alive_interval(20);
    conn_opts.set_clean_session(true);
    mqtt_client_->connect(conn_opts)->wait();

    RCLCPP_INFO(this->get_logger(), "Connected to private MQTT broker %s:%d",
      broker.c_str(), port);

    sub_task_ = this->create_subscription<robot_msgs::msg::TaskEvent>(
      "/task/event", 10,
      std::bind(&MqttPrivateBridge::task_callback, this, std::placeholders::_1));

    sub_chassis_ = this->create_subscription<robot_msgs::msg::ChassisStatus>(
      "/chassis/status", 10,
      std::bind(&MqttPrivateBridge::chassis_callback, this, std::placeholders::_1));

    sub_bms_ = this->create_subscription<robot_msgs::msg::BatteryStatus>(
      "/battery/status", 10,
      std::bind(&MqttPrivateBridge::bms_callback, this, std::placeholders::_1));

    pub_task_ros_ = this->create_publisher<robot_msgs::msg::TaskEvent>("/task/from_mqtt", 10);

    mqtt_client_->subscribe("cart/task", 1)->wait();

    RCLCPP_INFO(this->get_logger(), "comm_mqtt_private started");
  }

private:
  void on_message(mqtt::const_message_ptr msg) {
    if (msg->get_topic() == "cart/task") {
      try {
        auto j = json::parse(msg->to_string());
        auto ros_msg = robot_msgs::msg::TaskEvent();
        ros_msg.header.timestamp_ms = static_cast<int64_t>(this->now().seconds() * 1000);
        ros_msg.header.protocol_version = "1.3";
        ros_msg.task_id = j.value("task_id", "");
        ros_msg.order_id = j.value("order_id", "");
        ros_msg.event_version = j.value("event_version", 0u);
        pub_task_ros_->publish(ros_msg);
        RCLCPP_INFO(this->get_logger(), "MQTT→ROS task: %s", ros_msg.task_id.c_str());
      } catch (const std::exception & e) {
        RCLCPP_ERROR(this->get_logger(), "Failed to parse MQTT task: %s", e.what());
      }
    }
  }

  void task_callback(const robot_msgs::msg::TaskEvent::SharedPtr msg) {
    json j;
    j["task_id"] = msg->task_id;
    j["order_id"] = msg->order_id;
    j["event_version"] = msg->event_version;
    json actions = json::array();
    for (const auto & a : msg->actions) {
      actions.push_back({
        {"action_id", a.action_id},
        {"sku", a.sku},
        {"status", a.status},
        {"requested_qty", a.requested_qty},
        {"success_qty", a.success_qty},
      });
    }
    j["actions"] = actions;
    auto mqtt_msg = mqtt::make_message("cart/task/echo", j.dump());
    mqtt_client_->publish(mqtt_msg);
  }

  void chassis_callback(const robot_msgs::msg::ChassisStatus::SharedPtr msg) {
    json j;
    j["linear_x"] = msg->linear_x;
    j["angular_z"] = msg->angular_z;
    j["odometry_m"] = msg->odometry_m;
    j["mode"] = msg->mode;
    j["fault_code"] = msg->fault_code;
    j["estop"] = msg->estop;
    j["online"] = msg->online;
    auto mqtt_msg = mqtt::make_message("cart/chassis/status", j.dump());
    mqtt_client_->publish(mqtt_msg);
  }

  void bms_callback(const robot_msgs::msg::BatteryStatus::SharedPtr msg) {
    json j;
    j["soc"] = msg->soc;
    j["voltage"] = msg->voltage;
    j["current_a"] = msg->current_a;
    j["temperature"] = msg->temperature;
    j["is_charging"] = msg->is_charging;
    j["health"] = msg->health;
    auto mqtt_msg = mqtt::make_message("cart/battery/status", j.dump());
    mqtt_client_->publish(mqtt_msg);
  }

  std::shared_ptr<mqtt::async_client> mqtt_client_;
  std::unique_ptr<MqttCallbackHandler> callback_handler_;
  rclcpp::Subscription<robot_msgs::msg::TaskEvent>::SharedPtr sub_task_;
  rclcpp::Subscription<robot_msgs::msg::ChassisStatus>::SharedPtr sub_chassis_;
  rclcpp::Subscription<robot_msgs::msg::BatteryStatus>::SharedPtr sub_bms_;
  rclcpp::Publisher<robot_msgs::msg::TaskEvent>::SharedPtr pub_task_ros_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MqttPrivateBridge>());
  rclcpp::shutdown();
  return 0;
}
