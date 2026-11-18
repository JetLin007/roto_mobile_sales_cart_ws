# 协议 v1.3 §3.2.5 local/ui/heartbeat（APK→主控，每 2s，QoS0）
string ALIVE="ALIVE"

ProtocolHeader header
string status                # 固定 ALIVE
