#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/bms_status.hpp"
#include "robot_msgs/msg/gps_fix.hpp"
#include "robot_msgs/msg/chassis_state.hpp"
#include "std_msgs/msg/string.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @brief 状态汇总上报节点
 * 
 * 汇总 GPS/BMS/底盘/告警状态，定时输出 JSON 到 /status/report
 */
class StatusReportNode : public rclcpp::Node {
public:
  StatusReportNode() : Node("app_status_report") {
    this->declare_parameter("report_interval_ms", 3000);

    sub_bms_ = this->create_subscription<robot_msgs::msg::BmsStatus>(
      "/bms/status", 10,
      std::bind(&StatusReportNode::bms_callback, this, std::placeholders::_1));

    sub_gps_ = this->create_subscription<robot_msgs::msg::GpsFix>(
      "/gps/fix", 10,
      std::bind(&StatusReportNode::gps_callback, this, std::placeholders::_1));

    sub_chassis_ = this->create_subscription<robot_msgs::msg::ChassisState>(
      "/chassis/state", 10,
      std::bind(&StatusReportNode::chassis_callback, this, std::placeholders::_1));

    pub_report_ = this->create_publisher<std_msgs::msg::String>("/status/report", 10);

    int interval = static_cast<int>(get_parameter("report_interval_ms").as_int());
    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(interval),
      std::bind(&StatusReportNode::publish_report, this));

    RCLCPP_INFO(this->get_logger(), "app_status_report started, interval: %dms", interval);
  }

private:
  void bms_callback(const robot_msgs::msg::BmsStatus::SharedPtr msg) { latest_bms_ = *msg; }
  void gps_callback(const robot_msgs::msg::GpsFix::SharedPtr msg) { latest_gps_ = *msg; }
  void chassis_callback(const robot_msgs::msg::ChassisState::SharedPtr msg) { latest_chassis_ = *msg; }

  void publish_report() {
    json j;
    j["timestamp"] = this->now().seconds();
    j["bms"] = {
      {"soc", latest_bms_.soc},
      {"voltage", latest_bms_.voltage},
      {"charging", latest_bms_.charging},
      {"health", latest_bms_.health}
    };
    j["gps"] = {
      {"lat", latest_gps_.latitude},
      {"lon", latest_gps_.longitude},
      {"fix_type", latest_gps_.fix_type},
      {"satellites", latest_gps_.satellites}
    };
    j["chassis"] = {
      {"odom_x", latest_chassis_.odom_x},
      {"odom_y", latest_chassis_.odom_y},
      {"vel_linear", latest_chassis_.vel_linear},
      {"state", latest_chassis_.state}
    };

    std_msgs::msg::String msg;
    msg.data = j.dump();
    pub_report_->publish(msg);
  }

  rclcpp::Subscription<robot_msgs::msg::BmsStatus>::SharedPtr sub_bms_;
  rclcpp::Subscription<robot_msgs::msg::GpsFix>::SharedPtr sub_gps_;
  rclcpp::Subscription<robot_msgs::msg::ChassisState>::SharedPtr sub_chassis_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_report_;
  rclcpp::TimerBase::SharedPtr timer_;

  robot_msgs::msg::BmsStatus latest_bms_;
  robot_msgs::msg::GpsFix latest_gps_;
  robot_msgs::msg::ChassisState latest_chassis_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<StatusReportNode>());
  rclcpp::shutdown();
  return 0;
}