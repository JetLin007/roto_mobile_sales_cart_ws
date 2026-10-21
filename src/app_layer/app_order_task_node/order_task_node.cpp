#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/order_event.hpp"
#include "robot_msgs/msg/chassis_cmd.hpp"
#include "robot_msgs/msg/chassis_state.hpp"
#include "robot_msgs/msg/gps_fix.hpp"

/**
 * @brief 订单任务调度节点
 * 
 * 接收订单 → 解析目标点 → 控制底盘导航 → 到达后上报
 */
class OrderTaskNode : public rclcpp::Node {
public:
  OrderTaskNode() : Node("app_order_task") {
    sub_order_ = this->create_subscription<robot_msgs::msg::OrderEvent>(
      "/order/event", 10,
      std::bind(&OrderTaskNode::order_callback, this, std::placeholders::_1));

    sub_chassis_ = this->create_subscription<robot_msgs::msg::ChassisState>(
      "/chassis/state", 10,
      std::bind(&OrderTaskNode::chassis_callback, this, std::placeholders::_1));

    sub_gps_ = this->create_subscription<robot_msgs::msg::GpsFix>(
      "/gps/fix", 10,
      std::bind(&OrderTaskNode::gps_callback, this, std::placeholders::_1));

    pub_cmd_ = this->create_publisher<robot_msgs::msg::ChassisCmd>("/chassis/cmd", 10);

    RCLCPP_INFO(this->get_logger(), "app_order_task started");
  }

private:
  void order_callback(const robot_msgs::msg::OrderEvent::SharedPtr msg) {
    RCLCPP_INFO(this->get_logger(), "Order received: %s type=%d",
      msg->order_id.c_str(), msg->type);

    switch (msg->type) {
      case 1:  // START_DELIVERY
        target_lat_ = msg->target_lat;
        target_lon_ = msg->target_lon;
        navigating_ = true;
        RCLCPP_INFO(this->get_logger(), "Navigating to %.6f, %.6f", target_lat_, target_lon_);
        break;
      case 2:  // PAUSE
        navigating_ = false;
        send_stop();
        break;
      case 3:  // RESUME
        navigating_ = true;
        break;
      case 4:  // CANCEL
        navigating_ = false;
        current_order_id_.clear();
        send_stop();
        break;
      case 5:  // ARRIVED
        navigating_ = false;
        send_stop();
        RCLCPP_INFO(this->get_logger(), "Order %s completed", msg->order_id.c_str());
        break;
    }
    current_order_id_ = msg->order_id;
  }

  void chassis_callback(const robot_msgs::msg::ChassisState::SharedPtr msg) {
    current_state_ = *msg;
    if (navigating_) {
      navigate_to_target();
    }
  }

  void gps_callback(const robot_msgs::msg::GpsFix::SharedPtr msg) {
    current_gps_ = *msg;
  }

  void navigate_to_target() {
    // TODO: 替换为真实导航算法（如 move_base / nav2）
    // 这里用简单的 P 控制器示意
    if (current_gps_.latitude == 0) return;

    double lat_err = target_lat_ - current_gps_.latitude;
    double lon_err = target_lon_ - current_gps_.longitude;
    double dist = sqrt(lat_err * lat_err + lon_err * lon_err);

    if (dist < 0.00001) {  // ~1m 到达阈值
      navigating_ = false;
      send_stop();
      RCLCPP_INFO(this->get_logger(), "Arrived at target!");
      return;
    }

    auto cmd = robot_msgs::msg::ChassisCmd();
    cmd.header.stamp = this->now();
    cmd.linear_x = 0.5f;  // 匀速前进
    cmd.angular_z = static_cast<float>(atan2(lon_err, lat_err) * 0.1);
    cmd.gear = 1;
    pub_cmd_->publish(cmd);
  }

  void send_stop() {
    auto cmd = robot_msgs::msg::ChassisCmd();
    cmd.header.stamp = this->now();
    cmd.linear_x = 0;
    cmd.angular_z = 0;
    cmd.gear = 0;
    pub_cmd_->publish(cmd);
  }

  rclcpp::Subscription<robot_msgs::msg::OrderEvent>::SharedPtr sub_order_;
  rclcpp::Subscription<robot_msgs::msg::ChassisState>::SharedPtr sub_chassis_;
  rclcpp::Subscription<robot_msgs::msg::GpsFix>::SharedPtr sub_gps_;
  rclcpp::Publisher<robot_msgs::msg::ChassisCmd>::SharedPtr pub_cmd_;

  std::string current_order_id_;
  double target_lat_ = 0, target_lon_ = 0;
  bool navigating_ = false;
  robot_msgs::msg::ChassisState current_state_;
  robot_msgs::msg::GpsFix current_gps_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<OrderTaskNode>());
  rclcpp::shutdown();
  return 0;
}