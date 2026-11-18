# /alarm_light_cmd → driver_alarm_light
uint8 MODE_OFF=0
uint8 MODE_ON=1
uint8 MODE_BLINK=2
uint8 COLOR_RED=0
uint8 COLOR_YELLOW=1
uint8 COLOR_GREEN=2

ProtocolHeader header
uint8 mode
uint8 color
uint16 duration_sec          # 0=保持
