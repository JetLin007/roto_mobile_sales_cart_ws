# 协议 2.2.9 ota/app/version 与 2.2.10 ota/chassis/version 合并
uint8 TARGET_APP=0
uint8 TARGET_CHASSIS=1
uint8 DELIVERY_TAR=0
uint8 DELIVERY_PULL=1

ProtocolHeader header
string ota_task_id
uint8 target_module          # TARGET_*
string target_model          # CHASSIS 时如 STM32G474；APP 留空
string target_version
string url                   # image_url / firmware_url
string sha256                # 完整性校验
string signature             # 签名校验（与 sha256 分离，架构 E8）
uint8 delivery               # DELIVERY_*，CHASSIS 忽略
uint8 min_battery_soc        # APP 必填；CHASSIS 填 0
