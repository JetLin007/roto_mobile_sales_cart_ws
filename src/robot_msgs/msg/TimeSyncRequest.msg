# 协议 2.2.7 cmd/time_sync_req（上行）
ProtocolHeader header         # header.msg_id 即请求关联键
int64 t1                     # 车端发送时刻 UTC 毫秒
