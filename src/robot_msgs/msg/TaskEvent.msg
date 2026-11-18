# 协议 2.2.12 task/event 与 3.2.2 local/task/event 共用结构（全量动作快照 + 递增版本）
# 替代架构 §5.1 的 OrderEvent.msg：原定义粒度不足以表达 event_version 与数量账本
ProtocolHeader header
string task_id
string order_id           # 空=null
uint32 event_version      # 持久递增，云端/APK 只用更高版本覆盖
string query_msg_id       # 查询触发时关联 cmd/query 的 msg_id；空=非查询触发
TaskAction[] actions
