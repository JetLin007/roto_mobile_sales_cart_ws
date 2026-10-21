#include "ultrasonic_driver_base.hpp"
#include <random>

namespace driver_ultrasonic {

class UltrasonicStub : public UltrasonicDriverBase {
public:
  UltrasonicStub() : UltrasonicDriverBase("driver_ultrasonic_stub"), rng_(42), dist_(50, 200) {}

protected:
  std::pair<uint8_t, uint8_t> read_once() override {
    // 模拟前方 120cm、后方 150cm 无障碍
    uint8_t front = static_cast<uint8_t>(dist_(rng_));
    uint8_t rear = static_cast<uint8_t>(dist_(rng_));
    return {front, rear};
  }

private:
  std::mt19937 rng_;
  std::uniform_int_distribution<> dist_;
};

}  // namespace driver_ultrasonic

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<driver_ultrasonic::UltrasonicStub>());
  rclcpp::shutdown();
  return 0;
}