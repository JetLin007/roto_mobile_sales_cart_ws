# 1. 在宿主打包
bash scripts/ota_package.sh --version 1.1.0

# 2. 拿到输出的 MQTT JSON（脚本最后会打印），或者直接看 manifest
cat /opt/roto_ws/data/ota/cache/manifest-v1.1.0.json | python3 -c \
  "import sys,json; print(json.dumps(json.load(sys.stdin)['mqtt_payload'], indent=2))"

# 3. 把 tarball 跑个 HTTP server（供 ota_app curl 下载）
cd /opt/roto_ws/data/ota/cache && python3 -m http.server 18080 &

# 4. 用 mosquitto_pub 发 MQTT 指令（把上面拿到的 JSON 粘进去）
#    或者用 test_ota_full_loop.py 里的 bridge decode 路径