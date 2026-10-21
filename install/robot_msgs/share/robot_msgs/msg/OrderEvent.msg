# 订单事件（平板 → 车端）
std_msgs/Header header
string  order_id
string  tablet_id
uint8   type               # 1=START_DELIVERY, 2=PAUSE, 3=RESUME, 4=CANCEL, 5=ARRIVED
float64 target_lat
float64 target_lon
string  payload_json      # 订单详情 JSON