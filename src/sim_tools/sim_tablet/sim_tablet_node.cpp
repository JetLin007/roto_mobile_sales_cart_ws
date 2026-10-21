#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/order_event.hpp"
#include "std_srvs/srv/trigger.hpp"

/**
 * @brief WSL 仿真平板节点
 * 
 * 模拟平板发送订单事件到 /order/event
 * 用于测试 app_order_task 的完整流程
 */
class SimTabletNode : public rclcpp::Node {
public:
  SimTabletNode() : Node("sim_tablet"), order_count_(0) {
    this->declare_parameter("auto_send_interval_s", 10);
    this->declare_parameter("target_lat", 31.2350);
    this->declare_parameter("target_lon", 121.4800);

    pub_order_ = this->create_publisher<robot_msgs::msg::OrderEvent>("/order/event", 10);

    int interval = static_cast<int>(get_parameter("auto_send_interval_s").as_int());
    timer_ = this->create_wall_timer(
      std::chrono::seconds(interval),
      std::bind(&SimTabletNode::send_order, this));

    // 也提供手动触发服务
    srv_trigger_ = this->create_service<std_srvs::srv::Trigger>(
      "/sim_tablet/send_order",
      std::bind(&SimTabletNode::trigger_callback, this, std::placeholders::_1, std::placeholders::_2));

    RCLCPP_INFO(this->get_logger(), "sim_tablet started, auto-send every %ds", interval);
  }

private:
  void send_order() {
    auto msg = robot_msgs::msg::OrderEvent();
    msg.header.stamp = this->now();
    msg.order_id = "ORD_" + std::to_string(++order_count_);
    msg.tablet_id = "SIM_TABLET_01";
    msg.type = 1;  // START_DELIVERY
    msg.target_lat = get_parameter("target_lat").as_double();
    msg.target_lon = get_parameter("target_lon").as_double();
    msg.payload_json = "{\"items\":[{\"name\":\"Coffee\",\"qty\":2}]}";
    pub_order_->publish(msg);
    RCLCPP_INFO(this->get_logger(), "Sent order: %s", msg.order_id.c_str());
  }

    void trigger_callback(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> req,
    std::shared_ptr<std_srvs::srv::Trigger::Response> res)
  {
    (void)req;
    send_order();
    res->success = true;
    res->message = "Order sent";
  }

  rclcpp::Publisher<robot_msgs::msg::OrderEvent>::SharedPtr pub_order_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr srv_trigger_;
  rclcpp::TimerBase::SharedPtr timer_;
  int order_count_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SimTabletNode>());
  rclcpp::shutdown();
  return 0;
}