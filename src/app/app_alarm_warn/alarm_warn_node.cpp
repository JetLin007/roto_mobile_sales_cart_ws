#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/battery_status.hpp"
#include "robot_msgs/msg/chassis_status.hpp"
#include "robot_msgs/msg/alarm_event.hpp"
#include "std_msgs/msg/u_int8.hpp"

/**
 * @brief 告警管理节点
 *
 * 监控电池/底盘/超声波状态，异常时发布 AlarmEvent 到 /alarm
 */
class AlarmWarnNode : public rclcpp::Node {
public:
  AlarmWarnNode() : Node("app_alarm_warn") {
    this->declare_parameter("low_soc_threshold", 20);
    this->declare_parameter("high_temp_threshold", 60.0f);  // °C
    this->declare_parameter("ultrasonic_warn_cm", 30);

    sub_bms_ = this->create_subscription<robot_msgs::msg::BatteryStatus>(
      "/battery/status", 10,
      std::bind(&AlarmWarnNode::bms_callback, this, std::placeholders::_1));

    sub_chassis_ = this->create_subscription<robot_msgs::msg::ChassisStatus>(
      "/chassis/status", 10,
      std::bind(&AlarmWarnNode::chassis_callback, this, std::placeholders::_1));

    sub_ultrasonic_front_ = this->create_subscription<std_msgs::msg::UInt8>(
      "/ultrasonic/front", 10,
      std::bind(&AlarmWarnNode::ultrasonic_front_callback, this, std::placeholders::_1));

    sub_ultrasonic_rear_ = this->create_subscription<std_msgs::msg::UInt8>(
      "/ultrasonic/rear", 10,
      std::bind(&AlarmWarnNode::ultrasonic_rear_callback, this, std::placeholders::_1));

    pub_alarm_ = this->create_publisher<robot_msgs::msg::AlarmEvent>("/alarm", 10);

    RCLCPP_INFO(this->get_logger(), "app_alarm_warn started");
  }

private:
  void bms_callback(const robot_msgs::msg::BatteryStatus::SharedPtr msg) {
    int low_soc = static_cast<int>(get_parameter("low_soc_threshold").as_int());
    double high_temp = get_parameter("high_temp_threshold").as_double();

    if (msg->soc < low_soc) {
      publish_alarm("1001", robot_msgs::msg::AlarmEvent::LEVEL_WARNING,
        "driver_rs485_dev", "Low SOC: " + std::to_string(msg->soc) + "%");
    }
    if (msg->temperature > high_temp && msg->temperature > -127.0f) {
      publish_alarm("1002", robot_msgs::msg::AlarmEvent::LEVEL_CRITICAL,
        "driver_rs485_dev", "High temp: " + std::to_string(msg->temperature) + "C");
    }
  }

  void chassis_callback(const robot_msgs::msg::ChassisStatus::SharedPtr msg) {
    if (msg->mode == robot_msgs::msg::ChassisStatus::MODE_FAULT) {
      publish_alarm("2003", robot_msgs::msg::AlarmEvent::LEVEL_CRITICAL,
        "driver_chassis_rs485", "Chassis FAULT, code=" + std::to_string(msg->fault_code));
    }
  }

  void ultrasonic_front_callback(const std_msgs::msg::UInt8::SharedPtr msg) {
    int warn_cm = static_cast<int>(get_parameter("ultrasonic_warn_cm").as_int());
    if (msg->data > 0 && msg->data < static_cast<uint8_t>(warn_cm)) {
      publish_alarm("2001", robot_msgs::msg::AlarmEvent::LEVEL_WARNING,
        "driver_chassis_rs485", "Front obstacle: " + std::to_string(msg->data) + "cm");
    }
  }

  void ultrasonic_rear_callback(const std_msgs::msg::UInt8::SharedPtr msg) {
    int warn_cm = static_cast<int>(get_parameter("ultrasonic_warn_cm").as_int());
    if (msg->data > 0 && msg->data < static_cast<uint8_t>(warn_cm)) {
      publish_alarm("2002", robot_msgs::msg::AlarmEvent::LEVEL_WARNING,
        "driver_chassis_rs485", "Rear obstacle: " + std::to_string(msg->data) + "cm");
    }
  }

  void publish_alarm(const std::string & code, uint8_t level,
                     const std::string & module_id, const std::string & msg)
  {
    // 防抖：相同告警 5s 内不重复
    uint64_t key = (static_cast<uint64_t>(std::hash<std::string>{}(code)) << 32) | level;
    auto now = this->now().nanoseconds();
    auto it = last_alarm_time_.find(key);
    if (it != last_alarm_time_.end() && (now - it->second) < 5000000000LL) {
      return;  // 5s 内不重复
    }
    last_alarm_time_[key] = now;

    robot_msgs::msg::AlarmEvent alarm;
    alarm.header.timestamp_ms = static_cast<int64_t>(this->now().seconds() * 1000);
    alarm.header.protocol_version = "1.3";
    alarm.alarm_id = code + "_" + std::to_string(now);
    alarm.module_id = module_id;
    alarm.level = level;
    alarm.code = code;
    alarm.status = robot_msgs::msg::AlarmEvent::STATUS_ACTIVE;
    alarm.message = msg;
    pub_alarm_->publish(alarm);
    RCLCPP_WARN(this->get_logger(), "ALARM [%s] L%d: %s - %s",
      code.c_str(), level, module_id.c_str(), msg.c_str());
  }

  rclcpp::Subscription<robot_msgs::msg::BatteryStatus>::SharedPtr sub_bms_;
  rclcpp::Subscription<robot_msgs::msg::ChassisStatus>::SharedPtr sub_chassis_;
  rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr sub_ultrasonic_front_;
  rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr sub_ultrasonic_rear_;
  rclcpp::Publisher<robot_msgs::msg::AlarmEvent>::SharedPtr pub_alarm_;
  std::map<uint64_t, int64_t> last_alarm_time_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<AlarmWarnNode>());
  rclcpp::shutdown();
  return 0;
}
