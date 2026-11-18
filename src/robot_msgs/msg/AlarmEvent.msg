uint8 LEVEL_INFO=0
uint8 LEVEL_WARNING=1
uint8 LEVEL_CRITICAL=2
uint8 STATUS_ACTIVE=0
uint8 STATUS_CLEARED=1

# 协议 2.2.15 alarm/notify；云端按 alarm_id 去重与解除
ProtocolHeader header
string alarm_id
string module_id
uint8 level                  # LEVEL_*
string code                  # 如 DOOR_OPEN_TIMEOUT
uint8 status                 # STATUS_*
string message
