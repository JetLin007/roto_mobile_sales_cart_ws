#include "camera_driver_base.hpp"
#include <random>

namespace driver_camera {

class CameraStub : public CameraDriverBase {
public:
  CameraStub()
  : CameraDriverBase("driver_camera_stub"),
    width_(640), height_(480), rng_(42) {}

protected:
  sensor_msgs::msg::Image::SharedPtr capture_frame() override {
    auto img = std::make_shared<sensor_msgs::msg::Image>();
    img->height = height_;
    img->width = width_;
    img->encoding = "rgb8";
    img->is_bigendian = false;
    img->step = width_ * 3;
    img->data.resize(height_ * width_ * 3);
    // 填充随机颜色模拟画面
    std::uniform_int_distribution<> dist(0, 255);
    for (auto & b : img->data) b = static_cast<uint8_t>(dist(rng_));
    return img;
  }

  sensor_msgs::msg::CompressedImage::SharedPtr encode_h265(
    const sensor_msgs::msg::Image::SharedPtr & raw) override {
    // Stub: 模拟 H265 码流（实际是空数据）
    auto enc = std::make_shared<sensor_msgs::msg::CompressedImage>();
    enc->format = "h265";
    enc->data.resize(1024, 0);  // 模拟 1KB 码流
    return enc;
  }

private:
  int width_, height_;
  std::mt19937 rng_;
};

}  // namespace driver_camera

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<driver_camera::CameraStub>());
  rclcpp::shutdown();
  return 0;
}