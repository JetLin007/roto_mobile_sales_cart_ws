# 协议 2.2.8 cmd/time_sync_resp（下行）
# offset=((t2-t1)+(t3-t4))/2; delay=(t4-t1)-(t3-t2)
ProtocolHeader header         # header.msg_id 回填请求 msg_id
int64 t1
int64 t2
int64 t3
int64 t4                     # 车端收到响应的时刻
int64 server_time_utc
string timezone_offset       # 如 +08:00
string timezone_id           # 如 Asia/Shanghai
