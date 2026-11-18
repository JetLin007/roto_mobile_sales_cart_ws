#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/cmd_vel.hpp"
#include "robot_msgs/msg/chassis_status.hpp"
#include <cmath>

/**
 * @brief WSL 仿真底盘节点
 *
 * 接收 /cmd_vel → 积分运动学 → 发布 /chassis/status
 * 与 driver_chassis_rs485_stub 功能类似，但更完整用于系统测试
 */
class SimChassisNode : public rclcpp::Node {
public:
  SimChassisNode()
  : Node("sim_chassis"),
    x_(0), y_(0), theta_(0), vx_(0), wz_(0), mcu_heartbeat_(0) {
    sub_cmd_ = this->create_subscription<robot_msgs::msg::CmdVel>(
      "/cmd_vel", 10,
      std::bind(&SimChassisNode::cmd_callback, this, std::placeholders::_1));

    pub_state_ = this->create_publisher<robot_msgs::msg::ChassisStatus>("/chassis/status", 10);

    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(50),  // 20Hz
      std::bind(&SimChassisNode::publish_state, this));

    RCLCPP_INFO(this->get_logger(), "sim_chassis started");
  }

private:
  void cmd_callback(const robot_msgs::msg::CmdVel::SharedPtr msg) {
    if (msg->enable) {
      vx_ = msg->linear_x;
      wz_ = msg->angular_z;
    } else {
      vx_ = 0;
      wz_ = 0;
    }
  }

  void publish_state() {
    float dt = 0.05f;
    theta_ += wz_ * dt;
    x_ += vx_ * cosf(theta_) * dt;
    y_ += vx_ * sinf(theta_) * dt;
    odometry_m_ += std::fabs(vx_) * dt;

    robot_msgs::msg::ChassisStatus state;
    state.header.timestamp_ms = static_cast<int64_t>(this->now().seconds() * 1000);
    state.header.protocol_version = "1.3";
    state.linear_x = vx_;
    state.angular_z = wz_;
    state.odometry_m = odometry_m_;
    state.mcu_heartbeat = ++mcu_heartbeat_;
    state.mode = (vx_ != 0 || wz_ != 0)
      ? robot_msgs::msg::ChassisStatus::MODE_NORMAL
      : robot_msgs::msg::ChassisStatus::MODE_NORMAL;
    state.fault_code = 0;
    state.estop = false;
    state.online = true;
    pub_state_->publish(state);
  }

  rclcpp::Subscription<robot_msgs::msg::CmdVel>::SharedPtr sub_cmd_;
  rclcpp::Publisher<robot_msgs::msg::ChassisStatus>::SharedPtr pub_state_;
  rclcpp::TimerBase::SharedPtr timer_;
  float x_, y_, theta_, vx_, wz_;
  float odometry_m_ = 0.0f;
  uint16_t mcu_heartbeat_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SimChassisNode>());
  rclcpp::shutdown();
  return 0;
}
