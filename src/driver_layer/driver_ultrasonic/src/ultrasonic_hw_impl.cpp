#include "ultrasonic_driver_base.hpp"
#include <fcntl.h>
#include <unistd.h>

namespace driver_ultrasonic {

class UltrasonicHw : public UltrasonicDriverBase {
public:
    UltrasonicHw() : UltrasonicDriverBase("driver_ultrasonic_hw") {
    this->declare_parameter("front_gpio_pin", 17);
    this->declare_parameter("rear_gpio_pin", 27);
    RCLCPP_INFO(this->get_logger(), "driver_ultrasonic_hw GPIO pins: front=%ld, rear=%ld",
      this->get_parameter("front_gpio_pin").as_int(),
      this->get_parameter("rear_gpio_pin").as_int());
  }

protected:
  std::pair<uint8_t, uint8_t> read_once() override {
    // TODO: 通过 GPIO 触发超声波脉冲，测量回波时间计算距离
    // 或使用 I2C/SPI 接口的超声波模块
    return {0, 0};
  }
};

}  // namespace driver_ultrasonic

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<driver_ultrasonic::UltrasonicHw>());
  rclcpp::shutdown();
  return 0;
}