#include "camera_driver_base.hpp"
// #include <rockchip/rk_mpi.h>
// #include <rockchip/drmrga.h>

namespace driver_camera {

class CameraHw : public CameraDriverBase {
public:
  CameraHw() : CameraDriverBase("driver_camera_hw") {
    this->declare_parameter("camera_device", "/dev/video0");
    this->declare_parameter("width", 1920);
    this->declare_parameter("height", 1080);
    this->declare_parameter("fps", 30);
    this->declare_parameter("h265_bitrate_kbps", 2000);

    RCLCPP_INFO(this->get_logger(),
      "driver_camera_hw: %s @ %ldx%ld %ldfps, H265 %ldkbps",
      this->get_parameter("camera_device").as_string().c_str(),
      this->get_parameter("width").as_int(),
      this->get_parameter("height").as_int(),
      this->get_parameter("fps").as_int(),
      this->get_parameter("h265_bitrate_kbps").as_int());

    // TODO: 初始化 V4L2 摄像头设备
    // TODO: 初始化 RK MPP 编码器
    // TODO: 初始化 RK RGA 做色彩空间转换（NV12 → RGB）
  }

  ~CameraHw() override {
    // TODO: 释放 MPP/RGA/V4L2 资源
  }

protected:
  sensor_msgs::msg::Image::SharedPtr capture_frame() override {
    // TODO: V4L2 读取一帧 YUV/NV12
    // TODO: RGA 转换为 RGB
    return nullptr;
  }

  sensor_msgs::msg::CompressedImage::SharedPtr encode_h265(
    const sensor_msgs::msg::Image::SharedPtr & raw) override {
    // TODO: MPP 硬件编码 H265
    auto enc = std::make_shared<sensor_msgs::msg::CompressedImage>();
    enc->format = "h265";
    return enc;
  }
};

}  // namespace driver_camera

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<driver_camera::CameraHw>());
  rclcpp::shutdown();
  return 0;
}