# 协议 v1.3 §3.2.4 local/order/cancel（APK→主控）
# 仅为退出/取消 UI 意图；已支付订单的取消走云端 cmd/cancel
uint8 REASON_USER_CANCEL=0
uint8 REASON_TIMEOUT=1

ProtocolHeader header
string session_id
string order_id              # 空=null（尚未关联订单）
uint8 reason                 # REASON_*
