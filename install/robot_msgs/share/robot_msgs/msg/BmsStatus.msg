# 电池管理系统状态
std_msgs/Header header
uint8   soc                # 剩余电量 %
int16   voltage            # mV
int16   current            # mA, 正=放电 负=充电
int16   temp               # 0.1°C
bool    charging
uint8   health             # 0~100 %