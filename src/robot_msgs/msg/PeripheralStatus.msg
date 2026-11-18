# 单路 485 外设状态（/peripheral/{dev}/status）
uint8 STATE_UNKNOWN=0
uint8 STATE_ONLINE=1
uint8 STATE_OFFLINE=2
uint8 STATE_BUSY=3
uint8 STATE_FAULT=4

ProtocolHeader header
string module_id          # COFFEE-01 / MERCH-01 / FRIDGE-01 / BMS-01
string device_alias       # udev 别名，如 cart_485_coffee（架构 M3）
uint8 state               # STATE_*
bool online
float32 temperature       # 摄氏度；-127 表示无该传感器
string door_status        # CLOSED / OPEN / UNKNOWN
PeripheralMaterial[] materials
SkuStock[] sku_stock
string error_code         # 空=无错误
uint32 retry_count        # 驱动层累计重试次数（可观测性）
