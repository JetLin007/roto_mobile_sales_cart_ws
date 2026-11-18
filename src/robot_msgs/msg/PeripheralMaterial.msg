uint8 SOURCE_SENSOR=0
uint8 SOURCE_ESTIMATED=1
uint8 SOURCE_RFID=2
uint8 SOURCE_MANUAL=3

string material_id        # 如 BEANS / WATER / MILK / ICE
uint32 qty
string unit               # g / ml / pcs
uint8 source              # SOURCE_*（协议 2.2.13 materials.source）
