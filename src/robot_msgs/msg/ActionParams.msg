# 订单动作参数，1:1 对应协议 2.2.1 actions[].params（全部字段按需填，未用字段留默认值）
uint8 ICE_NORMAL=0
uint8 ICE_LESS=1
uint8 ICE_NO_ICE=2
uint8 TEMP_HOT=0
uint8 TEMP_COLD=1

string recipe_version     # 咖啡机配方版本，如 v2.1
string pickup_code        # 四位字符串，保留前导 0（如 "0088"）
uint16 qty                # 必填，>=1
uint8 sugar               # 0..3
uint8 ice                 # ICE_*
uint16 cup_ml             # 0=由 SKU/配方确定
uint8 temperature_mode    # TEMP_*，仅在无法由 SKU 确定时使用
uint16 duration_sec       # 柜体开门时长，非强制夹门时限
string[] authorized_rfid_list
string transaction_id     # 同柜多商品共用，物理只开门一次（协议 4.2）
