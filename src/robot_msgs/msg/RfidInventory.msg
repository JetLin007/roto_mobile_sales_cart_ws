# 协议 2.2.14 rfid/inventory
uint8 VR_MATCH=0
uint8 VR_PARTIAL_MATCH=1
uint8 VR_MISMATCH=2

ProtocolHeader header
string task_id
string action_id            # 组内主动作
string transaction_id
string cabinet_id
string[] baseline_tags
string[] current_tags
RfidDiff[] diff_tags
uint8 verification_result   # VR_*，仅盘点辅助判定，不单独触发退款
string[] action_ids         # 同柜关联的全部动作
bool is_final               # false=中间扫描，差异不稳定
