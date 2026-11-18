uint8 TARGET_COFFEE_MACHINE=0
uint8 TARGET_MERCHANDISE_CABINET=1
uint8 TARGET_ICE_CREAM_CABINET=2
uint8 TARGET_DRINK_FRIDGE=3
uint8 ACTION_MAKE=0
uint8 ACTION_UNLOCK=1

string action_id
string order_item_id
string sku
uint8 target              # TARGET_*
string module_id          # TARGET_COFFEE_MACHINE 时必填
string cabinet_id         # 柜体类目标必填
uint8 action_type         # ACTION_*
ActionParams params
