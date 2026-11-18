# 业务层 → 底盘 IO 指令（/chassis/io_cmd），仅转向灯/蜂鸣器，不含运动
ProtocolHeader header
bool turn_left
bool turn_right
bool buzzer
uint16 hold_ms            # 0=保持到下一次指令
