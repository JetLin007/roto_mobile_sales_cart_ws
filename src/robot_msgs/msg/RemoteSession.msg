# 远控会话事件（/remote/session）：架构 v1.2 §6.9 鉴权与保活、§9.2 远控安全
uint8 STATE_ENTERED=0
uint8 STATE_EXITED=1
uint8 STATE_REJECTED=2
uint8 STATE_EXPIRED=3

ProtocolHeader header
string control_session_id
string operator_id
string auth_ref              # 云端已授权引用，主控只做存在性校验，驾驶权由云端判定
uint8 state                  # STATE_*
int64 expire_at_ms           # 会话有效期；帧级 expire_at 必须更短（见 cart_config.remote）
string reason                # 拒绝/过期原因，如 STATE_MAKING、CHARGING、AUTH_MISSING
