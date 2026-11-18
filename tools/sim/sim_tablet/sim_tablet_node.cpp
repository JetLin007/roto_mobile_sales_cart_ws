#include "rclcpp/rclcpp.hpp"
#include "robot_msgs/msg/task_event.hpp"
#include "std_srvs/srv/trigger.hpp"

/**
 * @brief WSL 仿真平板节点
 *
 * 模拟平板发送任务事件到 /task/event
 * 用于测试 app_order_task 的完整流程
 */
class SimTabletNode : public rclcpp::Node {
public:
  SimTabletNode() : Node("sim_tablet"), order_count_(0), event_version_(0) {
    this->declare_parameter("auto_send_interval_s", 10);

    pub_order_ = this->create_publisher<robot_msgs::msg::TaskEvent>("/task/event", 10);

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
    auto msg = robot_msgs::msg::TaskEvent();
    msg.header.timestamp_ms = static_cast<int64_t>(this->now().seconds() * 1000);
    msg.header.protocol_version = "1.3";
    msg.task_id = "TASK_" + std::to_string(++order_count_);
    msg.order_id = "ORD_" + std::to_string(order_count_);
    msg.event_version = ++event_version_;

    // 单个动作：模拟一个出货任务
    robot_msgs::msg::TaskAction action;
    action.action_id = msg.task_id + "_A1";
    action.order_item_id = "ITEM_001";
    action.sku = "SKU_COFFEE";
    action.status = robot_msgs::msg::TaskAction::ST_QUEUED;
    action.requested_qty = 1;
    action.success_qty = 0;
    action.failed_qty = 0;
    action.cancelled_qty = 0;
    action.pending_qty = 1;
    action.unknown_qty = 0;
    action.is_final = false;
    action.unexecuted = true;
    action.progress = 0;
    action.estimated_remaining_sec = -1;
    msg.actions.push_back(action);

    pub_order_->publish(msg);
    RCLCPP_INFO(this->get_logger(), "Sent task: %s (order: %s)",
      msg.task_id.c_str(), msg.order_id.c_str());
  }

  void trigger_callback(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> req,
    std::shared_ptr<std_srvs::srv::Trigger::Response> res)
  {
    (void)req;
    send_order();
    res->success = true;
    res->message = "Task sent";
  }

  rclcpp::Publisher<robot_msgs::msg::TaskEvent>::SharedPtr pub_order_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr srv_trigger_;
  rclcpp::TimerBase::SharedPtr timer_;
  int order_count_;
  uint32_t event_version_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SimTabletNode>());
  rclcpp::shutdown();
  return 0;
}
