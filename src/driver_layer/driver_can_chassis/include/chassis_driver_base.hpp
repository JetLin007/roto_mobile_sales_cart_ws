#ifndef DRIVER_CAN_CHASSIS_BASE_HPP_
#define DRIVER_CAN_CHASSIS_BASE_HPP_

#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/chassis_cmd.hpp"
#include "robot_msgs/msg/chassis_state.hpp"

namespace driver_can_chassis {

/**
 * @brief CAN 底盘驱动基类
 * 
 * 订阅 /chassis/cmd → 下发 CAN 帧
 * 读取底盘 CAN 帧 → 发布 /chassis/state
 */
class ChassisDriverBase : public rclcpp::Node {
public:
  explicit ChassisDriverBase(const std::string & node_name)
  : rclcpp::Node(node_name) {
    pub_state_ = this->create_publisher<robot_msgs::msg::ChassisState>("/chassis/state", 10);
    sub_cmd_ = this->create_subscription<robot_msgs::msg::ChassisCmd>(
      "/chassis/cmd", 10,
      std::bind(&ChassisDriverBase::cmd_callback, this, std::placeholders::_1));
    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(50),  // 20Hz 反馈
      std::bind(&ChassisDriverBase::publish_state, this));
  }
  virtual ~ChassisDriverBase() = default;

protected:
  virtual void send_can_cmd(const robot_msgs::msg::ChassisCmd::SharedPtr cmd) = 0;
  virtual robot_msgs::msg::ChassisState read_chassis_state() = 0;

  void cmd_callback(const robot_msgs::msg::ChassisCmd::SharedPtr msg) {
    last_cmd_ = msg;
    send_can_cmd(msg);
  }

  robot_msgs::msg::ChassisCmd::SharedPtr last_cmd_;

private:
  void publish_state() {
    auto state = read_chassis_state();
    state.header.stamp = this->now();
    pub_state_->publish(state);
  }

  rclcpp::Publisher<robot_msgs::msg::ChassisState>::SharedPtr pub_state_;
  rclcpp::Subscription<robot_msgs::msg::ChassisCmd>::SharedPtr sub_cmd_;
  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace driver_can_chassis

#endif  // DRIVER_CAN_CHASSIS_BASE_HPP_