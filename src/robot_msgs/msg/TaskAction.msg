uint8 ST_QUEUED=0
uint8 ST_EXECUTING=1
uint8 ST_PRODUCED=2
uint8 ST_UNLOCKED=3
uint8 ST_DOOR_CLOSED=4
uint8 ST_PICKED_UP=5
uint8 ST_FAILED=6
uint8 ST_CANCELLED=7
uint8 ST_EXPIRED=8
uint8 ST_UNKNOWN=9

string action_id
string order_item_id
string sku
uint8 status                 # ST_*
uint16 requested_qty
uint16 success_qty
uint16 failed_qty
uint16 cancelled_qty
uint16 pending_qty
uint16 unknown_qty
bool is_final                # true 要求 pending_qty=unknown_qty=0（协议 4.1）
bool unexecuted
string pickup_code           # 空=null
string cabinet_id            # 空=null
string cabinet_label         # 如 A柜；空=null
string assigned_slot         # 空=null，界面显示“待分配”
string transaction_id        # 空=null
int32 progress               # 0..100；-1=null
string stage                 # 空=null
int32 estimated_remaining_sec # -1=null
string error_code            # 空=null
