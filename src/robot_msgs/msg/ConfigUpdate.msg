uint8 ACTION_UPDATE=0
uint8 ACTION_ROLLBACK=1

# 协议 2.2.6 config/update
ProtocolHeader header
string config_id
string version
uint8 action                 # ACTION_*
string payload_json          # 云端自由结构，校验后由 service_config 解析
string checksum              # 校验方式与签名要求由联调确认（协议 2.2.6）
