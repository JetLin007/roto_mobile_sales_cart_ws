# 外设单动作执行结果（driver_rs485_dev → app_order_task_node）
uint8 STATUS_QUEUED=0
uint8 STATUS_EXECUTING=1
uint8 STATUS_PRODUCED=2
uint8 STATUS_UNLOCKED=3
uint8 STATUS_DOOR_CLOSED=4
uint8 STATUS_PICKED_UP=5
uint8 STATUS_FAILED=6
uint8 STATUS_CANCELLED=7
uint8 STATUS_EXPIRED=8
uint8 STATUS_UNKNOWN=9

string action_id
string module_id
uint8 status              # STATUS_*
uint32 success_qty
uint32 failed_qty
string error_code         # 空=无错误
string message
int32 progress            # 0..100；-1=硬件无真实进度（协议 progress=null）
string stage              # 空=无
