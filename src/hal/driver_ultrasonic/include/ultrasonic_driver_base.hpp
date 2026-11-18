#ifndef DRIVER_ULTRASONIC_BASE_HPP_
#define DRIVER_ULTRASONIC_BASE_HPP_

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/u_int8.hpp"

namespace driver_ultrasonic {

/**
 * @brief 超声波驱动基类
 * 
 * 超声波数据独立发布到 /ultrasonic/front 和 /ultrasonic/rear
 * 底盘状态由 driver_chassis_rs485 通过 ChassisStatus 上报
 */
class UltrasonicDriverBase : public rclcpp::Node {
public:
  explicit UltrasonicDriverBase(const std::string & node_name)
  : rclcpp::Node(node_name) {
    pub_front_ = this->create_publisher<std_msgs::msg::UInt8>("/ultrasonic/front", 10);
    pub_rear_ = this->create_publisher<std_msgs::msg::UInt8>("/ultrasonic/rear", 10);
    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(100),  // 10Hz
      std::bind(&UltrasonicDriverBase::publish_loop, this));
  }
  virtual ~UltrasonicDriverBase() = default;

protected:
  virtual std::pair<uint8_t, uint8_t> read_once() = 0;  // {front_cm, rear_cm}

private:
  void publish_loop() {
    auto [front, rear] = read_once();
    std_msgs::msg::UInt8 msg_f;
    msg_f.data = front;
    pub_front_->publish(msg_f);

    std_msgs::msg::UInt8 msg_r;
    msg_r.data = rear;
    pub_rear_->publish(msg_r);
  }

  rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr pub_front_;
  rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr pub_rear_;
  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace driver_ultrasonic

#endif  // DRIVER_ULTRASONIC_BASE_HPP_