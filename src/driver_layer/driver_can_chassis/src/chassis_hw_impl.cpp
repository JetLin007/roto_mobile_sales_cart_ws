#include "chassis_driver_base.hpp"
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <cstring>

namespace driver_can_chassis {

class ChassisHw : public ChassisDriverBase {
public:
  ChassisHw() : ChassisDriverBase("driver_can_chassis_hw"), can_fd_(-1) {
    this->declare_parameter("can_interface", "can0");
    open_can(this->get_parameter("can_interface").as_string());
  }

  ~ChassisHw() override {
    if (can_fd_ >= 0) close(can_fd_);
  }

protected:
  void send_can_cmd(const robot_msgs::msg::ChassisCmd::SharedPtr cmd) override {
    if (can_fd_ < 0) return;
    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));
    frame.can_id = 0x200;  // 底盘控制 CAN ID
    frame.can_dlc = 8;
    // 简单映射：linear_x → byte0-1, angular_z → byte2-3, gear → byte4
    int16_t lin = static_cast<int16_t>(cmd->linear_x * 100);
    int16_t ang = static_cast<int16_t>(cmd->angular_z * 100);
    frame.data[0] = lin & 0xFF;
    frame.data[1] = (lin >> 8) & 0xFF;
    frame.data[2] = ang & 0xFF;
    frame.data[3] = (ang >> 8) & 0xFF;
    frame.data[4] = cmd->gear;
    write(can_fd_, &frame, sizeof(frame));
  }

  robot_msgs::msg::ChassisState read_chassis_state() override {
    // TODO: 从 SocketCAN 读取底盘反馈帧并解析
    robot_msgs::msg::ChassisState state;
    return state;
  }

private:
  void open_can(const std::string & iface) {
    can_fd_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (can_fd_ < 0) {
      RCLCPP_ERROR(this->get_logger(), "Failed to open CAN socket");
      return;
    }
    struct ifreq ifr;
    std::strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ - 1);
    ioctl(can_fd_, SIOCGIFINDEX, &ifr);
    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    bind(can_fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    RCLCPP_INFO(this->get_logger(), "CAN interface %s opened", iface.c_str());
  }

  int can_fd_;
};

}  // namespace driver_can_chassis

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<driver_can_chassis::ChassisHw>());
  rclcpp::shutdown();
  return 0;
}