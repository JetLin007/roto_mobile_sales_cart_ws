#ifndef DRIVER_CAMERA_BASE_HPP_
#define DRIVER_CAMERA_BASE_HPP_

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include "sensor_msgs/msg/image.hpp"

namespace driver_camera {

/**
 * @brief 摄像头驱动基类
 * 
 * 发布 /camera/raw（原始图像）和 /camera/h265（硬件编码码流）
 */
class CameraDriverBase : public rclcpp::Node {
public:
  explicit CameraDriverBase(const std::string & node_name)
  : rclcpp::Node(node_name) {
    pub_raw_ = this->create_publisher<sensor_msgs::msg::Image>("/camera/raw", 10);
    pub_h265_ = this->create_publisher<sensor_msgs::msg::CompressedImage>("/camera/h265", 10);
    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(33),  // ~30fps
      std::bind(&CameraDriverBase::capture_loop, this));
  }
  virtual ~CameraDriverBase() = default;

protected:
  virtual sensor_msgs::msg::Image::SharedPtr capture_frame() = 0;
  virtual sensor_msgs::msg::CompressedImage::SharedPtr encode_h265(
    const sensor_msgs::msg::Image::SharedPtr & raw) = 0;

  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr pub_raw_;
  rclcpp::Publisher<sensor_msgs::msg::CompressedImage>::SharedPtr pub_h265_;

private:
  void capture_loop() {
    auto raw = capture_frame();
    if (!raw) return;
    raw->header.stamp = this->now();
    pub_raw_->publish(*raw);

    auto encoded = encode_h265(raw);
    if (encoded) {
      encoded->header.stamp = this->now();
      pub_h265_->publish(*encoded);
    }
  }

  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace driver_camera

#endif  // DRIVER_CAMERA_BASE_HPP_