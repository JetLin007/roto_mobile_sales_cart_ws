#include "bms_driver_base.hpp"
#include <cmath>

namespace driver_bms {

class BmsStub : public BmsDriverBase {
public:
  BmsStub() : BmsDriverBase("driver_bms_stub"), t_(0.0) {
    this->declare_parameter("stub_initial_soc", 80);
    this->declare_parameter("stub_discharge_rate", 0.01);  // %/cycle
  }

protected:
  robot_msgs::msg::BmsStatus read_once() override {
    robot_msgs::msg::BmsStatus m;
    double init_soc = this->get_parameter("stub_initial_soc").as_double();
    double rate = this->get_parameter("stub_discharge_rate").as_double();

    // 模拟缓慢放电 + 正弦波动
    double soc = fmax(0.0, init_soc - rate * t_ + 2.0 * sin(t_ * 0.1));
    m.soc = static_cast<uint8_t>(soc);
    m.voltage = static_cast<int16_t>(48000 + 2000 * (soc / 100.0));  // 48V 系统
    m.current = static_cast<int16_t>(5000 + 1000 * sin(t_ * 0.05));  // 放电正电流
    m.temp = static_cast<int16_t>(250 + 30 * sin(t_ * 0.02));  // 25°C ±3°C
    m.charging = false;
    m.health = 95;

    // 低电量时模拟充电
    if (soc < 20.0) {
      m.charging = true;
      m.current = -3000;  // 充电负电流
    }

    t_ += 0.5;
    return m;
  }

private:
  double t_;
};

}  // namespace driver_bms

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<driver_bms::BmsStub>());
  rclcpp::shutdown();
  return 0;
}