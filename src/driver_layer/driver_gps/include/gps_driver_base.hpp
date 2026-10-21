#ifndef DRIVER_GPS_BASE_HPP_
#define DRIVER_GPS_BASE_HPP_

#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/gps_fix.hpp"

namespace driver_gps {

/**
 * @brief GPS 驱动基类，stub 和 hardware 都继承它
 * 
 * 统一对外发布 /gps/fix topic，子类只需实现 read_once()
 */
class GpsDriverBase : public rclcpp::Node {
public:
  explicit GpsDriverBase(const std::string & node_name)
  : rclcpp::Node(node_name) {
    pub_ = this->create_publisher<robot_msgs::msg::GpsFix>("/gps/fix", 10);
    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(100),  // 10Hz
      std::bind(&GpsDriverBase::publish_loop, this));
  }
  virtual ~GpsDriverBase() = default;

protected:
  /** @brief 子类实现：从硬件或仿真源读一帧数据 */
  virtual robot_msgs::msg::GpsFix read_once() = 0;

private:
  void publish_loop() {
    auto msg = read_once();
    msg.header.stamp = this->now();
    pub_->publish(msg);
  }

  rclcpp::Publisher<robot_msgs::msg::GpsFix>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace driver_gps

#endif  // DRIVER_GPS_BASE_HPP_