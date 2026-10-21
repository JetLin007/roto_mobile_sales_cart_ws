#include "bms_driver_base.hpp"
#include <fstream>
#include <sstream>

namespace driver_bms {

class BmsHw : public BmsDriverBase {
public:
  BmsHw() : BmsDriverBase("driver_bms_hw") {
    this->declare_parameter("can_interface", "can0");
    this->declare_parameter("bms_can_id", 0x180);
    RCLCPP_INFO(this->get_logger(), "driver_bms_hw on %s, CAN ID 0x%X",
      this->get_parameter("can_interface").as_string().c_str(),
      static_cast<unsigned>(this->get_parameter("bms_can_id").as_int()));
  }

protected:
  robot_msgs::msg::BmsStatus read_once() override {
    // TODO: 从 SocketCAN 读取 BMS CAN 帧并解析
    // candump can0 | grep 180
    robot_msgs::msg::BmsStatus m;
    return m;
  }
};

}  // namespace driver_bms

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<driver_bms::BmsHw>());
  rclcpp::shutdown();
  return 0;
}