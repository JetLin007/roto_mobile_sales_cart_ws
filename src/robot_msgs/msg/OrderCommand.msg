# 协议 2.2.1 cmd/order → /order/command（service_cloud_comm → app_order_task_node）
ProtocolHeader header
string order_id
string task_id
int64 expire_at_ms        # 各动作最晚启动时刻；过期未启动不得执行
string pickup_code        # 整单取货码，四位字符串
OrderAction[] actions
