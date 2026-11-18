# 底盘状态（RS485 上行，20ms 周期内的 1 次上报；对应 /chassis/status）
uint8 MODE_NORMAL=0
uint8 MODE_OTA=1
uint8 MODE_FAULT=2

ProtocolHeader header
float32 linear_x          # m/s，底盘实际反馈速度
float32 angular_z         # rad/s
float32 odometry_m        # 累计里程，m
uint16 mcu_heartbeat      # STM32 递增心跳计数，driver 侧据此判定 MCU 在线
uint8 mode                # MODE_*
uint32 fault_code         # STM32 故障码，0=无故障
bool estop                # 急停硬线状态（架构 §4.4 安全状态机输入）
bool online               # 60ms（3 帧）内是否收到有效应答
