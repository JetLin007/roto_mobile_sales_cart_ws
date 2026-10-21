#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/bms_status.hpp"
#include "robot_msgs/msg/chassis_state.hpp"
#include "robot_msgs/msg/alarm.hpp"

/**
 * @brief 告警管理节点
 * 
 * 监控 BMS/底盘/GPS 状态，异常时发布告警到 /alarm
 */
class AlarmWarnNode : public rclcpp::Node {
public:
  AlarmWarnNode() : Node("app_alarm_warn") {
    this->declare_parameter("low_soc_threshold", 20);
    this->declare_parameter("high_temp_threshold", 600);  // 60.0°C (0.1°C 单位)
    this->declare_parameter("ultrasonic_warn_cm", 30);

    sub_bms_ = this->create_subscription<robot_msgs::msg::BmsStatus>(
      "/bms/status", 10,
      std::bind(&AlarmWarnNode::bms_callback, this, std::placeholders::_1));

    sub_chassis_ = this->create_subscription<robot_msgs::msg::ChassisState>(
      "/chassis/state", 10,
      std::bind(&AlarmWarnNode::chassis_callback, this, std::placeholders::_1));

    pub_alarm_ = this->create_publisher<robot_msgs::msg::Alarm>("/alarm", 10);

    RCLCPP_INFO(this->get_logger(), "app_alarm_warn started");
  }

private:
  void bms_callback(const robot_msgs::msg::BmsStatus::SharedPtr msg) {
    int low_soc = static_cast<int>(get_parameter("low_soc_threshold").as_int());
    int high_temp = static_cast<int>(get_parameter("high_temp_threshold").as_int());

    if (msg->soc < low_soc) {
      publish_alarm(1001, 2, "driver_bms", "Low SOC: " + std::to_string(msg->soc) + "%");
    }
    if (msg->temp > high_temp) {
      publish_alarm(1002, 3, "driver_bms", "High temp: " + std::to_string(msg->temp / 10.0) + "°C");
    }
  }

  void chassis_callback(const robot_msgs::msg::ChassisState::SharedPtr msg) {
    int warn_cm = static_cast<int>(get_parameter("ultrasonic_warn_cm").as_int());

    if (msg->ultrasonic_front < warn_cm && msg->ultrasonic_front > 0) {
      publish_alarm(2001, 2, "driver_can_chassis",
        "Front obstacle: " + std::to_string(msg->ultrasonic_front) + "cm");
    }
    if (msg->ultrasonic_rear < warn_cm && msg->ultrasonic_rear > 0) {
      publish_alarm(2002, 2, "driver_can_chassis",
        "Rear obstacle: " + std::to_string(msg->ultrasonic_rear) + "cm");
    }
    if (msg->state == 2) {  // ERROR
      publish_alarm(2003, 3, "driver_can_chassis", "Chassis ERROR");
    }
  }

  void publish_alarm(uint16_t code, uint8_t level, const std::string & source, const std::string & msg) {
    // 防抖：相同告警 5s 内不重复
    uint64_t key = (static_cast<uint64_t>(code) << 32) | source.size();
    auto now = this->now().nanoseconds();
    auto it = last_alarm_time_.find(key);
    if (it != last_alarm_time_.end() && (now - it->second) < 5000000000LL) {
      return;  // 5s 内不重复
    }
    last_alarm_time_[key] = now;

    robot_msgs::msg::Alarm alarm;
    alarm.header.stamp = this->now();
    alarm.alarm_code = code;
    alarm.level = level;
    alarm.source = source;
    alarm.message = msg;
    alarm.cleared = false;
    pub_alarm_->publish(alarm);
    RCLCPP_WARN(this->get_logger(), "ALARM [%d] L%d: %s - %s", code, level, source.c_str(), msg.c_str());
  }

  rclcpp::Subscription<robot_msgs::msg::BmsStatus>::SharedPtr sub_bms_;
  rclcpp::Subscription<robot_msgs::msg::ChassisState>::SharedPtr sub_chassis_;
  rclcpp::Publisher<robot_msgs::msg::Alarm>::SharedPtr pub_alarm_;
  std::map<uint64_t, int64_t> last_alarm_time_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<AlarmWarnNode>());
  rclcpp::shutdown();
  return 0;
}