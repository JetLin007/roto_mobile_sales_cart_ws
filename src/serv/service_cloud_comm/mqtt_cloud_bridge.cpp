#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/chassis_status.hpp"
#include "robot_msgs/msg/battery_status.hpp"
#include "robot_msgs/msg/alarm_event.hpp"
#include "robot_msgs/msg/geo_location.hpp"
#include "std_msgs/msg/string.hpp"
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

class MqttCloudBridge : public rclcpp::Node {
public:
  MqttCloudBridge() : Node("comm_mqtt_cloud") {
    this->declare_parameter("cloud_broker", "mqtt.example.com");
    this->declare_parameter("cloud_port", 8883);
    this->declare_parameter("cloud_username", "cart001");
    this->declare_parameter("cloud_password", "");
    this->declare_parameter("device_id", "cart_001");
    this->declare_parameter("report_interval_ms", 5000);

    std::string broker = get_parameter("cloud_broker").as_string();
    int port = static_cast<int>(get_parameter("cloud_port").as_int());
    std::string username = get_parameter("cloud_username").as_string();
    std::string password = get_parameter("cloud_password").as_string();
    device_id_ = get_parameter("device_id").as_string();
    int interval = static_cast<int>(get_parameter("report_interval_ms").as_int());

    std::string uri = "ssl://" + broker + ":" + std::to_string(port);
    mqtt_client_ = std::make_shared<mqtt::async_client>(uri, device_id_);

    callback_handler_ = std::make_unique<MqttCallbackHandler>(
      [this](mqtt::const_message_ptr msg) {
        RCLCPP_DEBUG(this->get_logger(), "Cloud MQTT msg: %s", msg->get_topic().c_str());
      },
      [this](const std::string & cause) {
        RCLCPP_WARN(this->get_logger(), "Cloud MQTT lost: %s", cause.c_str());
      });
    mqtt_client_->set_callback(*callback_handler_);

    mqtt::connect_options opts;
    opts.set_user_name(username);
    opts.set_password(password);
    opts.set_keep_alive_interval(30);
    opts.set_clean_session(true);
    mqtt::ssl_options ssl_opts;
    opts.set_ssl(ssl_opts);
    mqtt_client_->connect(opts)->wait();

    RCLCPP_INFO(this->get_logger(), "Connected to cloud MQTT %s", uri.c_str());

    sub_bms_ = this->create_subscription<robot_msgs::msg::BatteryStatus>(
      "/battery/status", 10,
      std::bind(&MqttCloudBridge::bms_callback, this, std::placeholders::_1));

    sub_gps_ = this->create_subscription<robot_msgs::msg::GeoLocation>(
      "/gps/fix", 10,
      std::bind(&MqttCloudBridge::gps_callback, this, std::placeholders::_1));

    sub_alarm_ = this->create_subscription<robot_msgs::msg::AlarmEvent>(
      "/alarm", 10,
      std::bind(&MqttCloudBridge::alarm_callback, this, std::placeholders::_1));

    sub_chassis_ = this->create_subscription<robot_msgs::msg::ChassisStatus>(
      "/chassis/status", 10,
      std::bind(&MqttCloudBridge::chassis_callback, this, std::placeholders::_1));

    pub_report_ = this->create_publisher<std_msgs::msg::String>("/status/report", 10);

    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(interval),
      std::bind(&MqttCloudBridge::periodic_report, this));

    RCLCPP_INFO(this->get_logger(), "comm_mqtt_cloud started, report interval: %dms", interval);
  }

private:
  void bms_callback(const robot_msgs::msg::BatteryStatus::SharedPtr msg) { latest_bms_ = *msg; }
  void gps_callback(const robot_msgs::msg::GeoLocation::SharedPtr msg) { latest_gps_ = *msg; }
  void chassis_callback(const robot_msgs::msg::ChassisStatus::SharedPtr msg) { latest_chassis_ = *msg; }

  void alarm_callback(const robot_msgs::msg::AlarmEvent::SharedPtr msg) {
    json j;
    j["device_id"] = device_id_;
    j["type"] = "alarm";
    j["alarm_id"] = msg->alarm_id;
    j["module_id"] = msg->module_id;
    j["level"] = msg->level;
    j["code"] = msg->code;
    j["status"] = msg->status;
    j["message"] = msg->message;
    j["timestamp"] = this->now().seconds();
    publish_mqtt("cart/alarm", j);
  }

  void periodic_report() {
    if (latest_bms_.soc != 0 || latest_gps_.latitude != 0) {
      json j;
      j["device_id"] = device_id_;
      j["type"] = "status";
      j["soc"] = latest_bms_.soc;
      j["voltage"] = latest_bms_.voltage;
      j["charging"] = latest_bms_.is_charging;
      j["lat"] = latest_gps_.latitude;
      j["lon"] = latest_gps_.longitude;
      j["speed"] = latest_gps_.speed_mps;
      j["chassis"] = {
        {"linear_x", latest_chassis_.linear_x},
        {"angular_z", latest_chassis_.angular_z},
        {"odometry_m", latest_chassis_.odometry_m},
        {"mode", latest_chassis_.mode},
        {"fault_code", latest_chassis_.fault_code},
        {"online", latest_chassis_.online}
      };
      j["timestamp"] = this->now().seconds();
      publish_mqtt("cart/status", j);
    }

    // 本地 ROS 汇总上报（原 app_status_report 职责）
    json local;
    local["timestamp"] = this->now().seconds();
    local["bms"] = {
      {"soc", latest_bms_.soc},
      {"voltage", latest_bms_.voltage},
      {"charging", latest_bms_.is_charging},
      {"health", latest_bms_.health}
    };
    local["gps"] = {
      {"lat", latest_gps_.latitude},
      {"lon", latest_gps_.longitude},
      {"valid", latest_gps_.valid},
      {"satellites", latest_gps_.satellites}
    };
    local["chassis"] = {
      {"linear_x", latest_chassis_.linear_x},
      {"angular_z", latest_chassis_.angular_z},
      {"odometry_m", latest_chassis_.odometry_m},
      {"mode", latest_chassis_.mode}
    };
    std_msgs::msg::String report_msg;
    report_msg.data = local.dump();
    pub_report_->publish(report_msg);
  }

  void publish_mqtt(const std::string & topic, const json & j) {
    try {
      auto msg = mqtt::make_message(topic, j.dump());
      msg->set_qos(1);
      mqtt_client_->publish(msg);
    } catch (const std::exception & e) {
      RCLCPP_ERROR(this->get_logger(), "MQTT publish failed: %s", e.what());
    }
  }

  std::shared_ptr<mqtt::async_client> mqtt_client_;
  std::unique_ptr<MqttCallbackHandler> callback_handler_;
  std::string device_id_;
  rclcpp::Subscription<robot_msgs::msg::BatteryStatus>::SharedPtr sub_bms_;
  rclcpp::Subscription<robot_msgs::msg::GeoLocation>::SharedPtr sub_gps_;
  rclcpp::Subscription<robot_msgs::msg::AlarmEvent>::SharedPtr sub_alarm_;
  rclcpp::Subscription<robot_msgs::msg::ChassisStatus>::SharedPtr sub_chassis_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_report_;
  rclcpp::TimerBase::SharedPtr timer_;
  robot_msgs::msg::BatteryStatus latest_bms_;
  robot_msgs::msg::GeoLocation latest_gps_;
  robot_msgs::msg::ChassisStatus latest_chassis_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MqttCloudBridge>());
  rclcpp::shutdown();
  return 0;
}
