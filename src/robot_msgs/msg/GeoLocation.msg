# 位置上报（云端 2.2.13 location，以及 /gps/fix 的补充信息）
# 说明：/gps/fix 走 sensor_msgs/NavSatFix 标准消息；NavSatFix 无速度与卫星数字段，
#      因此 speed_mps / satellites / hdop / vdop 由本消息承载
bool valid                   # false → 协议 location=null
float64 latitude             # deg
float64 longitude            # deg
float32 altitude_m           # m
float32 heading_deg          # 0~360，正北为 0
float32 speed_mps            # m/s（存量仓库 GpsFix 原字段，迁入此处）
uint8 satellites             # 可见卫星数（存量仓库 GpsFix 原字段，迁入此处）
float32 hdop                 # 水平精度因子（同上）
float32 vdop                 # 垂直精度因子（同上）
