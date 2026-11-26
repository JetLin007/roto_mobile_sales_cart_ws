#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/chassis_status.hpp"
#include "robot_msgs/msg/battery_status.hpp"
#include "robot_msgs/msg/alarm_event.hpp"
#include "robot_msgs/msg/geo_location.hpp"
#include "robot_msgs/msg/ota_command.hpp"
#include "robot_msgs/msg/ota_status.hpp"
#include "service_cloud_comm/core/ota_codec.hpp"
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
    this->declare_parameter("ota_app_topic", "ota/app/version");
    this->declare_parameter("ota_chassis_topic", "ota/chassis/version");
    this->declare_parameter("ota_status_topic", "ota/status");

    std::string broker = get_parameter("cloud_broker").as_string();
    int port = static_cast<int>(get_parameter("cloud_port").as_int());
    std::string username = get_parameter("cloud_username").as_string();
    std::string password = get_parameter("cloud_password").as_string();
    device_id_ = get_parameter("device_id").as_string();
    int interval = static_cast<int>(get_parameter("report_interval_ms").as_int());
    ota_app_topic_ = get_parameter("ota_app_topic").as_string();
    ota_chassis_topic_ = get_parameter("ota_chassis_topic").as_string();
    ota_status_topic_ = get_parameter("ota_status_topic").as_string();

    // 先建好 ROS 侧 OTA 通道，避免 MQTT 回调早于发布者创建而空指针
    pub_ota_cmd_ = this->create_publisher<robot_msgs::msg::OtaCommand>("/ota/command", 10);
    sub_ota_status_ = this->create_subscription<robot_msgs::msg::OtaStatus>(
      "/ota/status", 10,
      std::bind(&MqttCloudBridge::ota_status_callback, this, std::placeholders::_1));

    std::string uri = "ssl://" + broker + ":" + std::to_string(port);
    mqtt_client_ = std::make_shared<mqtt::async_client>(uri, device_id_);

    callback_handler_ = std::make_unique<MqttCallbackHandler>(
      [this](mqtt::const_message_ptr msg) { this->on_mqtt_message(msg); },
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

    // 订阅 OTA 下行指令（协议 v1.3 §2.2.9 / §2.2.10）
    mqtt_client_->subscribe(ota_app_topic_, 1)->wait();
    mqtt_client_->subscribe(ota_chassis_topic_, 1)->wait();

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
  // 云端 MQTT 下行消息分发（paho 回调线程，转发到 ROS 是线程安全的）
  void on_mqtt_message(mqtt::const_message_ptr msg) {
    const std::string topic = msg->get_topic();
    if (topic != ota_app_topic_ && topic != ota_chassis_topic_) {
      RCLCPP_DEBUG(this->get_logger(), "Cloud MQTT msg ignored: %s", topic.c_str());
      return;
    }

    json j = json::parse(msg->to_string(), nullptr, false);
    if (j.is_discarded()) {
      RCLCPP_ERROR(this->get_logger(), "OTA payload is not valid JSON on %s", topic.c_str());
      return;
    }
    auto cmd = robot_msgs::msg::OtaCommand();
    std::string err;
    if (!service_cloud_comm::core::decode_ota_command(j, device_id_, cmd, err)) {
      // 契约不合法：丢弃并记错，不把半成品指令下发给 ota_app
      RCLCPP_ERROR(this->get_logger(), "OTA command rejected on %s: %s",
                   topic.c_str(), err.c_str());
      return;
    }
    pub_ota_cmd_->publish(cmd);
    RCLCPP_INFO(this->get_logger(), "OTA command forwarded: task=%s module=%u ver=%s",
                cmd.ota_task_id.c_str(), cmd.target_module, cmd.target_version.c_str());
  }

  // OtaStatus -> ota/status JSON
  void ota_status_callback(const robot_msgs::msg::OtaStatus::SharedPtr msg) {
    publish_mqtt(ota_status_topic_, service_cloud_comm::core::encode_ota_status(*msg));
  }

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
  std::string ota_app_topic_;
  std::string ota_chassis_topic_;
  std::string ota_status_topic_;
  rclcpp::Publisher<robot_msgs::msg::OtaCommand>::SharedPtr pub_ota_cmd_;
  rclcpp::Subscription<robot_msgs::msg::OtaStatus>::SharedPtr sub_ota_status_;
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
