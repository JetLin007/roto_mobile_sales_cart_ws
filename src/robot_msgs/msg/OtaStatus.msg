uint8 TARGET_APP=0
uint8 TARGET_CHASSIS=1
uint8 STAGE_DOWNLOADING=0
uint8 STAGE_VERIFYING=1
uint8 STAGE_INSTALLING=2
uint8 STAGE_ROLLBACK=3
uint8 STAGE_FINISHED=4
uint8 RESULT_NONE=0
uint8 RESULT_SUCCESS=1
uint8 RESULT_FAILED=2
uint8 RESULT_ROLLBACK_SUCCESS=3
uint8 RESULT_ROLLBACK_FAILED=4

# 协议 2.2.16 ota/status
ProtocolHeader header
string ota_task_id
uint8 target_module          # TARGET_*
string target_version
uint8 stage                  # STAGE_*
uint8 progress               # 0..100
uint8 result                 # RESULT_*，非终态填 RESULT_NONE
