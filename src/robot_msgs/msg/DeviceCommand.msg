# 协议 2.2.5 cmd/device
uint8 CMD_SET_TEMPERATURE=0
uint8 CMD_FORCE_UNLOCK=1
uint8 CMD_STOP=2
uint8 CMD_ENTER_REMOTE=3
uint8 CMD_EXIT_REMOTE=4
uint8 UNIT_C=0
uint8 UNIT_F=1

ProtocolHeader header
string operator_id        # 审计标识，不等于驾驶权
string target             # CHASSIS / COFFEE_MACHINE / MERCHANDISE_CABINET / DRINK_FRIDGE
string module_id
uint8 command             # CMD_*
string control_session_id # ENTER_REMOTE 必填
string auth_ref           # ENTER_REMOTE 必填，已授权引用
float32 target_temp       # 0 表示未设置
uint8 unit                # UNIT_*
int64 expire_at_ms        # ENTER_REMOTE 必填；0=不适用
