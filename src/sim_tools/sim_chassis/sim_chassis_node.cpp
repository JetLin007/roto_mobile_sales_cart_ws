#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/chassis_cmd.hpp"
#include "robot_msgs/msg/chassis_state.hpp"
#include <cmath>

/**
 * @brief WSL 仿真底盘节点
 * 
 * 接收 /chassis/cmd → 积分运动学 → 发布 /chassis/state
 * 与 driver_can_chassis_stub 功能类似，但更完整用于系统测试
 */
class SimChassisNode : public rclcpp::Node {
public:
  SimChassisNode()
  : Node("sim_chassis"),
    x_(0), y_(0), theta_(0), vx_(0), wz_(0) {
    sub_cmd_ = this->create_subscription<robot_msgs::msg::ChassisCmd>(
      "/chassis/cmd", 10,
      std::bind(&SimChassisNode::cmd_callback, this, std::placeholders::_1));

    pub_state_ = this->create_publisher<robot_msgs::msg::ChassisState>("/chassis/state", 10);

    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(50),  // 20Hz
      std::bind(&SimChassisNode::publish_state, this));

    RCLCPP_INFO(this->get_logger(), "sim_chassis started");
  }

private:
  void cmd_callback(const robot_msgs::msg::ChassisCmd::SharedPtr msg) {
    vx_ = msg->linear_x;
    wz_ = msg->angular_z;
  }

  void publish_state() {
    float dt = 0.05f;
    theta_ += wz_ * dt;
    x_ += vx_ * cosf(theta_) * dt;
    y_ += vx_ * sinf(theta_) * dt;

    robot_msgs::msg::ChassisState state;
    state.header.stamp = this->now();
    state.odom_x = x_;
    state.odom_y = y_;
    state.odom_theta = theta_;
    state.vel_linear = vx_;
    state.vel_angular = wz_;
    state.state = (vx_ != 0 || wz_ != 0) ? 1 : 0;
    state.ultrasonic_front = 100 + static_cast<uint8_t>(20 * sin(this->now().seconds()));
    state.ultrasonic_rear = 120 + static_cast<uint8_t>(15 * cos(this->now().seconds()));
    pub_state_->publish(state);
  }

  rclcpp::Subscription<robot_msgs::msg::ChassisCmd>::SharedPtr sub_cmd_;
  rclcpp::Publisher<robot_msgs::msg::ChassisState>::SharedPtr pub_state_;
  rclcpp::TimerBase::SharedPtr timer_;
  float x_, y_, theta_, vx_, wz_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SimChassisNode>());
  rclcpp::shutdown();
  return 0;
}