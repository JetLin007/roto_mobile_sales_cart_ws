# 告警消息
std_msgs/Header header
uint16  alarm_code        # 告警码
uint8   level             # 1=INFO 2=WARN 3=ERROR 4=FATAL
string  source            # 触发源 (e.g. "driver_bms")
string  message
bool    cleared