#include "chassis_driver_base.hpp"
#include <cmath>

namespace driver_can_chassis {

class ChassisStub : public ChassisDriverBase {
public:
  ChassisStub()
  : ChassisDriverBase("driver_can_chassis_stub"),
    odom_x_(0.0f), odom_y_(0.0f), odom_theta_(0.0f),
    vel_linear_(0.0f), vel_angular_(0.0f) {}

protected:
  void send_can_cmd(const robot_msgs::msg::ChassisCmd::SharedPtr cmd) override {
    // Stub: 简单积分模拟运动学
    vel_linear_ = cmd->linear_x;
    vel_angular_ = cmd->angular_z;
    RCLCPP_DEBUG(this->get_logger(), "Stub cmd: lin=%.2f ang=%.2f gear=%d",
      vel_linear_, vel_angular_, cmd->gear);
  }

  robot_msgs::msg::ChassisState read_chassis_state() override {
    float dt = 0.05f;  // 20Hz
    odom_theta_ += vel_angular_ * dt;
    odom_x_ += vel_linear_ * cosf(odom_theta_) * dt;
    odom_y_ += vel_linear_ * sinf(odom_theta_) * dt;

    robot_msgs::msg::ChassisState state;
    state.odom_x = odom_x_;
    state.odom_y = odom_y_;
    state.odom_theta = odom_theta_;
    state.vel_linear = vel_linear_;
    state.vel_angular = vel_angular_;
    state.state = (vel_linear_ != 0.0f || vel_angular_ != 0.0f) ? 1 : 0;  // RUNNING or IDLE
    state.ultrasonic_front = 120;  // 模拟固定值
    state.ultrasonic_rear = 150;
    return state;
  }

private:
  float odom_x_, odom_y_, odom_theta_;
  float vel_linear_, vel_angular_;
};

}  // namespace driver_can_chassis

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<driver_can_chassis::ChassisStub>());
  rclcpp::shutdown();
  return 0;
}