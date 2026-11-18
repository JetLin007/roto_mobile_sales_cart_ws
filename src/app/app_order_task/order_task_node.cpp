#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/task_event.hpp"
#include "robot_msgs/msg/chassis_status.hpp"
#include "robot_msgs/msg/geo_location.hpp"
#include "robot_msgs/msg/cmd_vel.hpp"
#include <cmath>

/**
 * @brief 订单任务调度节点
 *
 * 接收任务事件 → 管理任务状态 → 控制底盘导航 → 到达后上报
 */
class OrderTaskNode : public rclcpp::Node {
public:
  OrderTaskNode() : Node("app_order_task") {
    sub_task_ = this->create_subscription<robot_msgs::msg::TaskEvent>(
      "/task/event", 10,
      std::bind(&OrderTaskNode::task_callback, this, std::placeholders::_1));

    sub_chassis_ = this->create_subscription<robot_msgs::msg::ChassisStatus>(
      "/chassis/status", 10,
      std::bind(&OrderTaskNode::chassis_callback, this, std::placeholders::_1));

    sub_gps_ = this->create_subscription<robot_msgs::msg::GeoLocation>(
      "/gps/fix", 10,
      std::bind(&OrderTaskNode::gps_callback, this, std::placeholders::_1));

    pub_cmd_ = this->create_publisher<robot_msgs::msg::CmdVel>("/cmd_vel", 10);

    RCLCPP_INFO(this->get_logger(), "app_order_task started");
  }

private:
  void task_callback(const robot_msgs::msg::TaskEvent::SharedPtr msg) {
    RCLCPP_INFO(this->get_logger(), "Task received: %s (order: %s)",
      msg->task_id.c_str(), msg->order_id.c_str());

    current_task_id_ = msg->task_id;

    // 判断任务状态（取第一个动作的状态）
    if (!msg->actions.empty()) {
      const auto & action = msg->actions[0];
      switch (action.status) {
        case robot_msgs::msg::TaskAction::ST_QUEUED:
        case robot_msgs::msg::TaskAction::ST_EXECUTING:
          navigating_ = true;
          RCLCPP_INFO(this->get_logger(), "Navigating for task %s", current_task_id_.c_str());
          break;
        case robot_msgs::msg::TaskAction::ST_CANCELLED:
        case robot_msgs::msg::TaskAction::ST_FAILED:
        case robot_msgs::msg::TaskAction::ST_EXPIRED:
          navigating_ = false;
          current_task_id_.clear();
          send_stop();
          break;
        case robot_msgs::msg::TaskAction::ST_PRODUCED:
        case robot_msgs::msg::TaskAction::ST_UNLOCKED:
        case robot_msgs::msg::TaskAction::ST_DOOR_CLOSED:
        case robot_msgs::msg::TaskAction::ST_PICKED_UP:
          // 任务完成，停止导航
          navigating_ = false;
          current_task_id_.clear();
          send_stop();
          RCLCPP_INFO(this->get_logger(), "Task %s completed", msg->task_id.c_str());
          break;
        default:
          break;
      }
    }
  }

  void chassis_callback(const robot_msgs::msg::ChassisStatus::SharedPtr msg) {
    current_state_ = *msg;
    if (navigating_) {
      navigate_forward();
    }
  }

  void gps_callback(const robot_msgs::msg::GeoLocation::SharedPtr msg) {
    current_gps_ = *msg;
  }

  void navigate_forward() {
    // TODO: 替换为真实导航算法（如 move_base / nav2）
    // 这里用简单的匀速前进示意
    auto cmd = robot_msgs::msg::CmdVel();
    cmd.header.timestamp_ms = static_cast<int64_t>(this->now().seconds() * 1000);
    cmd.header.protocol_version = "1.3";
    cmd.linear_x = 0.5f;
    cmd.angular_z = 0.0f;
    cmd.enable = true;
    cmd.source = robot_msgs::msg::CmdVel::SRC_LOCAL;
    pub_cmd_->publish(cmd);
  }

  void send_stop() {
    auto cmd = robot_msgs::msg::CmdVel();
    cmd.header.timestamp_ms = static_cast<int64_t>(this->now().seconds() * 1000);
    cmd.header.protocol_version = "1.3";
    cmd.linear_x = 0;
    cmd.angular_z = 0;
    cmd.enable = false;
    cmd.source = robot_msgs::msg::CmdVel::SRC_LOCAL;
    pub_cmd_->publish(cmd);
  }

  rclcpp::Subscription<robot_msgs::msg::TaskEvent>::SharedPtr sub_task_;
  rclcpp::Subscription<robot_msgs::msg::ChassisStatus>::SharedPtr sub_chassis_;
  rclcpp::Subscription<robot_msgs::msg::GeoLocation>::SharedPtr sub_gps_;
  rclcpp::Publisher<robot_msgs::msg::CmdVel>::SharedPtr pub_cmd_;

  std::string current_task_id_;
  bool navigating_ = false;
  robot_msgs::msg::ChassisStatus current_state_;
  robot_msgs::msg::GeoLocation current_gps_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<OrderTaskNode>());
  rclcpp::shutdown();
  return 0;
}
