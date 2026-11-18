# 协议 2.2.4 cmd/motion（QoS0）。仅当前会话递增 seq 且未过期的帧有效；
# 500ms 无有效新帧 → app_alarm_warn_node 必须下发 CmdVel(enable=false)
ProtocolHeader header
string control_session_id
uint32 seq                 # >=1，单调递增
int64 expire_at_ms         # UTC 毫秒，过期帧丢弃
float32 linear_x           # -1.39..1.39 m/s
float32 angular_z          # rad/s
bool enable
