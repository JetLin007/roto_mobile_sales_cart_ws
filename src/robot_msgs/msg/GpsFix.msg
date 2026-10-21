# GPS 定位数据
std_msgs/Header header
float64 latitude          # 纬度 (deg)
float64 longitude         # 经度 (deg)
float32 altitude          # 海拔 (m)
float32 speed             # m/s
float32 heading           # 航向角 (deg, 0~360)
uint8   fix_type          # 0=no fix, 1=GPS, 2=DGPS, 4=RTK, 5=RTK float
uint8   satellites
float32 hdop
float32 vdop