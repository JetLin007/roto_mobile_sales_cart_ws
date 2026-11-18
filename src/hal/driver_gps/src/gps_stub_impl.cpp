#include "gps_driver_base.hpp"
#include <cmath>

namespace driver_gps {

class GpsStub : public GpsDriverBase {
public:
  GpsStub() : GpsDriverBase("driver_gps_stub"), t_(0.0) {
    // 模拟一个缓慢沿直线移动的点
    this->declare_parameter("stub_base_lat", 31.2304);
    this->declare_parameter("stub_base_lon", 121.4737);
  }

protected:
  robot_msgs::msg::GeoLocation read_once() override {
    robot_msgs::msg::GeoLocation m;
    double base_lat = this->get_parameter("stub_base_lat").as_double();
    double base_lon = this->get_parameter("stub_base_lon").as_double();
    // 模拟匀速向北移动
    m.latitude  = base_lat + 0.00001 * t_;
    m.longitude = base_lon;
    m.altitude_m  = 10.0f;
    m.speed_mps   = 1.5f;
    m.heading_deg = 0.0f;
    m.valid       = true;   // 假装 RTK 固定解
    m.satellites  = 12;
    m.hdop        = 0.6f;
    m.vdop        = 0.8f;
    t_ += 0.1;
    return m;
  }

private:
  double t_;
};

}  // namespace driver_gps

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<driver_gps::GpsStub>());
  rclcpp::shutdown();
  return 0;
}