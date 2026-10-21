#ifndef DRIVER_GPIO_BASE_HPP_
#define DRIVER_GPIO_BASE_HPP_

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_srvs/srv/set_bool.hpp"

namespace driver_gpio {

/**
 * @brief GPIO 驱动基类
 * 
 * 提供 /gpio/set 服务控制 GPIO 输出
 * 发布 /gpio/state 反馈当前状态
 */
class GpioDriverBase : public rclcpp::Node {
public:
  explicit GpioDriverBase(const std::string & node_name)
  : rclcpp::Node(node_name) {
    pub_state_ = this->create_publisher<std_msgs::msg::Bool>("/gpio/state", 10);
    srv_set_ = this->create_service<std_srvs::srv::SetBool>(
      "/gpio/set",
      std::bind(&GpioDriverBase::set_callback, this, std::placeholders::_1, std::placeholders::_2));
    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(1000),
      std::bind(&GpioDriverBase::publish_state, this));
  }
  virtual ~GpioDriverBase() = default;

protected:
  virtual void set_gpio(bool value) = 0;
  virtual bool read_gpio() = 0;

  void set_callback(
    const std::shared_ptr<std_srvs::srv::SetBool::Request> req,
    std::shared_ptr<std_srvs::srv::SetBool::Response> res)
  {
    set_gpio(req->data);
    res->success = true;
    res->message = req->data ? "GPIO ON" : "GPIO OFF";
  }

private:
  void publish_state() {
    std_msgs::msg::Bool msg;
    msg.data = read_gpio();
    pub_state_->publish(msg);
  }

  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr pub_state_;
  rclcpp::Service<std_srvs::srv::SetBool>::SharedPtr srv_set_;
  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace driver_gpio

#endif  // DRIVER_GPIO_BASE_HPP_