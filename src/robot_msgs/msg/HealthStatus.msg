# /metrics 与容器健康探针（架构 §9.3 / Q3）
uint8 STATE_STARTING=0
uint8 STATE_READY=1
uint8 STATE_DEGRADED=2
uint8 STATE_FAILED=3

ProtocolHeader header
string node_name
uint8 state
float32 cpu_percent
uint32 mem_mb
uint32 queue_depth
uint32 device_timeout_count
string detail
