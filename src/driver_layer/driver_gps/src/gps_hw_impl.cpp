#include "gps_driver_base.hpp"
#include <fstream>
#include <sstream>
// 真实实现里这里换成串口库，比如 serial/serial.h

namespace driver_gps {

class GpsHw : public GpsDriverBase {
public:
  GpsHw() : GpsDriverBase("driver_gps_hw") {
    this->declare_parameter("serial_port", "/dev/ttyUSB0");
    this->declare_parameter("baud_rate", 115200);
    // serial_port_ = new Serial(this->get_parameter("serial_port").as_string(), ...);
    RCLCPP_INFO(this->get_logger(), "driver_gps_hw opened %s @ %d",
      this->get_parameter("serial_port").as_string().c_str(),
      this->get_parameter("baud_rate").as_int());
  }
  ~GpsHw() override {
    // delete serial_port_;
  }

protected:
  robot_msgs::msg::GpsFix read_once() override {
    // TODO: 解析 NMEA 或 GNSS 二进制协议
    robot_msgs::msg::GpsFix m;
    // 真实代码从 serial_port_ 读一行 $GNRMC ... 并解析
    return m;
  }

private:
  // Serial * serial_port_;
};

}  // namespace driver_gps

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<driver_gps::GpsHw>());
  rclcpp::shutdown();
  return 0;
}