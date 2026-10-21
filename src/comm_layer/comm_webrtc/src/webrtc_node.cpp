#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include <fstream>
#include <sstream>

/**
 * @brief WebRTC 媒体封装节点
 * 
 * 订阅 /camera/h265 → 通过 WebRTC DataChannel 推流到浏览器
 * 通过 MQTT 做信令交换（SDP/ICE）
 * 
 * 技术选型建议：
 *   - 轻量方案：libdatachannel（C++ WebRTC 库，~2MB）
 *   - 完整方案：GStreamer + webrtcbin
 *   - 最简单：Janus Gateway / mediasoup 做 SFU，车端只推 RTSP
 */
class WebRtcNode : public rclcpp::Node {
public:
  WebRtcNode() : Node("comm_webrtc") {
    this->declare_parameter("signaling_mqtt_broker", "127.0.0.1");
    this->declare_parameter("signaling_mqtt_port", 1883);
    this->declare_parameter("webrtc_port", 8443);
    this->declare_parameter("stream_id", "cart_camera_001");

    sub_h265_ = this->create_subscription<sensor_msgs::msg::CompressedImage>(
      "/camera/h265", 10,
      std::bind(&WebRtcNode::h265_callback, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "comm_webrtc started, stream_id: %s",
      get_parameter("stream_id").as_string().c_str());

    // TODO: 初始化 WebRTC PeerConnection
    // TODO: 创建 DataChannel / VideoTrack
    // TODO: 连接信令 MQTT，交换 SDP/ICE Candidate
  }

private:
  void h265_callback(const sensor_msgs::msg::CompressedImage::SharedPtr msg) {
    // TODO: 将 H265 NALU 送入 WebRTC VideoTrack
    (void)msg;
  }

  rclcpp::Subscription<sensor_msgs::msg::CompressedImage>::SharedPtr sub_h265_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<WebRtcNode>());
  rclcpp::shutdown();
  return 0;
}