# 底盘 IO 状态（/chassis/io_status）
ProtocolHeader header
bool estop                # 急停
bool turn_left            # 左转向灯执行状态
bool turn_right           # 右转向灯执行状态
bool buzzer_active
uint32 io_fault_code
