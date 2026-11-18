# 协议 2.2.11 cmd/ack
uint8 ACK_RECEIVED=0
uint8 ACK_EXECUTING=1
uint8 ACK_SUCCESS=2
uint8 ACK_FAILED=3
uint8 QR_NONE=0
uint8 QR_FOUND=1
uint8 QR_NOT_FOUND=2
uint8 QR_UNAVAILABLE=3

ProtocolHeader header
string ack_msg_id          # 对应原请求 msg_id
string ack_topic           # 如 cmd/order、cmd/query、cmd/cancel、cmd/device
uint8 status               # ACK_*
string error_code          # 空=null
string task_id             # 空=null
uint8 query_result         # QR_*，非查询应答填 QR_NONE
bool retryable             # 仅未受理拒绝且有效期内的重试
bool unexecuted
CancelResult[] cancel_results
