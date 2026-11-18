# 架构 v1.2 §6.6 唯一权威状态集（8 状态）+ 协议 v1.3 §2.2.13 state/report
# 序列化规则：system_state=STATE_OFFLINE 时只发 LWT 小 JSON（msg_id/timestamp/cart_id/system_state），
# 其余字段一律不填；正常遥测报文禁止出现 STATE_OFFLINE
uint8 STATE_INIT=0
uint8 STATE_IDLE=1
uint8 STATE_ORDERING=2
uint8 STATE_MAKING=3
uint8 STATE_REMOTE_DRIVE=4
uint8 STATE_OTA=5
uint8 STATE_FAULT=6
uint8 STATE_OFFLINE=7

ProtocolHeader header
string cart_id
uint8 system_state           # STATE_*
BatteryStatus battery
InventoryModule[] inventory
GeoLocation location
ChassisHealth chassis
PeripheralHealth[] peripherals
