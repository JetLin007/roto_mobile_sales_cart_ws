#include "gpio_driver_base.hpp"

namespace driver_gpio {

class GpioStub : public GpioDriverBase {
public:
  GpioStub() : GpioDriverBase("driver_gpio_stub"), gpio_value_(false) {}

protected:
  void set_gpio(bool value) override {
    gpio_value_ = value;
    RCLCPP_INFO(this->get_logger(), "Stub GPIO set to %s", value ? "ON" : "OFF");
  }

  bool read_gpio() override {
    return gpio_value_;
  }

private:
  bool gpio_value_;
};

}  // namespace driver_gpio

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<driver_gpio::GpioStub>());
  rclcpp::shutdown();
  return 0;
}