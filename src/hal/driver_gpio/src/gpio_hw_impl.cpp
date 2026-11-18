#include "gpio_driver_base.hpp"
#include <cinttypes>
// 实车使用 libgpiod: #include <gpiod.h>

namespace driver_gpio {

class GpioHw : public GpioDriverBase {
public:
  GpioHw() : GpioDriverBase("driver_gpio_hw") {
    this->declare_parameter("output_pin", 4);
    this->declare_parameter("input_pin", 17);
    RCLCPP_INFO(this->get_logger(),
      "driver_gpio_hw pins: out=%" PRId64 ", in=%" PRId64,
      this->get_parameter("output_pin").as_int(),
      this->get_parameter("input_pin").as_int());
    // TODO: gpiod_chip_open / gpiod_line_request_output
  }

protected:
  void set_gpio(bool value) override {
    // TODO: gpiod_line_set_value
    (void)value;
  }

  bool read_gpio() override {
    // TODO: gpiod_line_get_value
    return false;
  }
};

}  // namespace driver_gpio

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<driver_gpio::GpioHw>());
  rclcpp::shutdown();
  return 0;
}