# /voice_play → driver_audio；节点内按优先级插播（防撞告警最高）
uint8 PRIORITY_ALARM=0
uint8 PRIORITY_HIGH=1
uint8 PRIORITY_NORMAL=2

ProtocolHeader header
uint8 priority
string text                  # TTS 文本
string audio_uri             # 预录音频；空=走 TTS
