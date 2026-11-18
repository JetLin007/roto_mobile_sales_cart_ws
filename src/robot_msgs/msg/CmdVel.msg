# 指令仲裁输出 → driver_chassis_rs485（/cmd_vel）
# 优先级：SRC_ALARM(防撞限速/急停) > SRC_RC(2.4G 遥控) > SRC_CLOUD(云端远控) > SRC_LOCAL(本地业务)
uint8 SRC_NONE=0
uint8 SRC_LOCAL=1
uint8 SRC_CLOUD=2
uint8 SRC_RC=3
uint8 SRC_ALARM=4

ProtocolHeader header
float32 linear_x              # m/s，正=前进 负=后退（替代存量 gear 档位）
float32 linear_y              # m/s，麦轮横移；差速底盘恒为 0（存量仓库保留字段）
float32 angular_z             # rad/s，正=逆时针
bool enable                   # false=停车（协议 cmd/motion enable=false 同义）
uint8 source                  # SRC_*
string control_session_id     # 云端远控会话（协议 2.2.4），本地/遥控为空
uint32 seq                    # 会话内递增序号，驱动侧丢弃非递增帧
int64 expire_at_ms            # UTC 毫秒；业务层 500ms 无新有效帧须主动下发 enable=false
string note                   # 仲裁说明（限速原因/急停来源），便于日志追溯
