# 全车统一消息头（对应协议 v1.3 各 Payload 公共字段）
# 时间基准：UTC 毫秒（架构 P10），禁止本地时间；主机由 GNSS + chrony 维持
string msg_id            # 全局唯一，同时作为幂等键；ACK 用 ack_msg_id 关联本字段
int64 timestamp_ms       # UTC 毫秒
string protocol_version  # 固定 "1.3"；按主版本兼容，同主版本新增字段可忽略（架构 M14）
string cart_id           # 整车标识，如 cart-0000001
