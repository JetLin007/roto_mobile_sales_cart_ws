#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/order_event.hpp"
#include "robot_msgs/msg/chassis_state.hpp"
#include "robot_msgs/msg/bms_status.hpp"
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

    sub_order_ = this->create_subscription<robot_msgs::msg::OrderEvent>(
      "/order/event", 10,
      std::bind(&MqttPrivateBridge::order_callback, this, std::placeholders::_1));

    sub_chassis_ = this->create_subscription<robot_msgs::msg::ChassisState>(
      "/chassis/state", 10,
      std::bind(&MqttPrivateBridge::chassis_callback, this, std::placeholders::_1));

    sub_bms_ = this->create_subscription<robot_msgs::msg::BmsStatus>(
      "/bms/status", 10,
      std::bind(&MqttPrivateBridge::bms_callback, this, std::placeholders::_1));

    pub_order_ros_ = this->create_publisher<robot_msgs::msg::OrderEvent>("/order/from_mqtt", 10);

    mqtt_client_->subscribe("cart/order", 1)->wait();

    RCLCPP_INFO(this->get_logger(), "comm_mqtt_private started");
  }

private:
  void on_message(mqtt::const_message_ptr msg) {
    if (msg->get_topic() == "cart/order") {
      try {
        auto j = json::parse(msg->to_string());
        auto ros_msg = robot_msgs::msg::OrderEvent();
        ros_msg.header.stamp = this->now();
        ros_msg.order_id = j.value("order_id", "");
        ros_msg.tablet_id = j.value("tablet_id", "");
        ros_msg.type = j.value("type", 0);
        ros_msg.target_lat = j.value("target_lat", 0.0);
        ros_msg.target_lon = j.value("target_lon", 0.0);
        ros_msg.payload_json = j.dump();
        pub_order_ros_->publish(ros_msg);
        RCLCPP_INFO(this->get_logger(), "MQTT→ROS order: %s", ros_msg.order_id.c_str());
      } catch (const std::exception & e) {
        RCLCPP_ERROR(this->get_logger(), "Failed to parse MQTT order: %s", e.what());
      }
    }
  }

  void order_callback(const robot_msgs::msg::OrderEvent::SharedPtr msg) {
    json j;
    j["order_id"] = msg->order_id;
    j["tablet_id"] = msg->tablet_id;
    j["type"] = msg->type;
    j["target_lat"] = msg->target_lat;
    j["target_lon"] = msg->target_lon;
    j["payload"] = msg->payload_json;
    auto mqtt_msg = mqtt::make_message("cart/order/echo", j.dump());
    mqtt_client_->publish(mqtt_msg);
  }

  void chassis_callback(const robot_msgs::msg::ChassisState::SharedPtr msg) {
    json j;
    j["odom_x"] = msg->odom_x;
    j["odom_y"] = msg->odom_y;
    j["odom_theta"] = msg->odom_theta;
    j["vel_linear"] = msg->vel_linear;
    j["vel_angular"] = msg->vel_angular;
    j["state"] = msg->state;
    j["ultrasonic_front"] = msg->ultrasonic_front;
    j["ultrasonic_rear"] = msg->ultrasonic_rear;
    auto mqtt_msg = mqtt::make_message("cart/chassis/state", j.dump());
    mqtt_client_->publish(mqtt_msg);
  }

  void bms_callback(const robot_msgs::msg::BmsStatus::SharedPtr msg) {
    json j;
    j["soc"] = msg->soc;
    j["voltage"] = msg->voltage;
    j["current"] = msg->current;
    j["temp"] = msg->temp;
    j["charging"] = msg->charging;
    j["health"] = msg->health;
    auto mqtt_msg = mqtt::make_message("cart/bms/status", j.dump());
    mqtt_client_->publish(mqtt_msg);
  }

  std::shared_ptr<mqtt::async_client> mqtt_client_;
  std::unique_ptr<MqttCallbackHandler> callback_handler_;
  rclcpp::Subscription<robot_msgs::msg::OrderEvent>::SharedPtr sub_order_;
  rclcpp::Subscription<robot_msgs::msg::ChassisState>::SharedPtr sub_chassis_;
  rclcpp::Subscription<robot_msgs::msg::BmsStatus>::SharedPtr sub_bms_;
  rclcpp::Publisher<robot_msgs::msg::OrderEvent>::SharedPtr pub_order_ros_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MqttPrivateBridge>());
  rclcpp::shutdown();
  return 0;
}