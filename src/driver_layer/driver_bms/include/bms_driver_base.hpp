#ifndef DRIVER_BMS_BASE_HPP_
#define DRIVER_BMS_BASE_HPP_

#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/bms_status.hpp"

namespace driver_bms {

class BmsDriverBase : public rclcpp::Node {
public:
  explicit BmsDriverBase(const std::string & node_name)
  : rclcpp::Node(node_name) {
    pub_ = this->create_publisher<robot_msgs::msg::BmsStatus>("/bms/status", 10);
    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(500),  // 2Hz
      std::bind(&BmsDriverBase::publish_loop, this));
  }
  virtual ~BmsDriverBase() = default;

protected:
  virtual robot_msgs::msg::BmsStatus read_once() = 0;

private:
  void publish_loop() {
    auto msg = read_once();
    msg.header.stamp = this->now();
    pub_->publish(msg);
  }

  rclcpp::Publisher<robot_msgs::msg::BmsStatus>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace driver_bms

#endif  // DRIVER_BMS_BASE_HPP_