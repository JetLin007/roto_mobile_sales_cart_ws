uint8 MODE_NORMAL=0
uint8 MODE_OTA=1
uint8 MODE_FAULT=2

# driver_chassis_rs485 模式机唯一权威广播（NORMAL/OTA_MODE 互斥，架构 E4）
ProtocolHeader header
uint8 mode
uint32 reason_code
string reason
