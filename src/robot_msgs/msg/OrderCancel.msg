uint8 REASON_CLOUD_CANCEL=0
uint8 REASON_USER_CANCEL=1

# 协议 2.2.2 cmd/cancel → /order/cancel
ProtocolHeader header
string task_id
string[] action_ids       # 空数组=取消整任务
uint8 reason              # REASON_*
