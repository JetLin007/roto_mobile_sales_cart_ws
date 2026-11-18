# 协议 v1.3 §3.2.1 local/cart/status（主控→APK）；不承载 OFFLINE，屏幕离线由心跳判定
uint8 STATE_INIT=0
uint8 STATE_IDLE=1
uint8 STATE_ORDERING=2
uint8 STATE_MAKING=3
uint8 STATE_REMOTE_DRIVE=4
uint8 STATE_OTA=5
uint8 STATE_FAULT=6

ProtocolHeader header
uint8 state
string message               # 面向用户的中文提示，如“车辆处于远程维护状态，暂停新下单”
