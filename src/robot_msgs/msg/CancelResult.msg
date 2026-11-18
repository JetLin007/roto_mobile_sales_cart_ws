uint8 RESULT_CANCELLED=0
uint8 RESULT_ALREADY_CANCELLED=1
uint8 RESULT_TOO_LATE=2
uint8 RESULT_UNKNOWN=3
uint8 RESULT_TOMBSTONED=4

string action_id          # 空=整任务级取消（协议 action_id=null）
uint8 result              # RESULT_*
bool unexecuted           # 仅在有记录证明未发生物理动作时为 true
