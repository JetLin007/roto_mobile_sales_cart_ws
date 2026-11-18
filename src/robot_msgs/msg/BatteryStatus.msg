# 电池状态（/battery/status，ROS 内部消息，无 ProtocolHeader）
# 存量仓库字段迁移说明：原 int16 voltage(mV) → float32 voltage(V)；
#                     原 int16 temp(0.1°C) → float32 temperature(°C)；
#                     原 bool charging → bool is_charging
uint8 SOURCE_BMS=0
uint8 SOURCE_ESTIMATED=1

uint8 soc                    # 0..100，剩余电量百分比
float32 voltage              # V（伏特；原 BmsStatus 为 mV，已统一为 V）
float32 current_a            # A，正=放电 负=充电（存量仓库扩展字段，用于充放电判定）
float32 temperature          # 摄氏度；-127=未知（原 0.1°C 已改）
bool is_charging             # 充电中；CHARGING 状态由此派生，不进 CartState 枚举
uint8 health                 # 0..100 %，0xFF=未知（存量仓库扩展字段）
uint8 source                 # SOURCE_*
