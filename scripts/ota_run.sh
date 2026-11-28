#!/usr/bin/env bash
# =============================================================================
# ota_run.sh — 端到端 OTA 执行脚本（生产/测试都能用）
# -----------------------------------------------------------------------------
# 从 ota_package.sh 产物 manifest.json 读取所有信息，完成：
#
#   1) 解析 manifest.json → tarball / sha256 / signature / 公钥
#   2) 起 HTTP server（供 ota_app curl 下载 tarball）
#   3) 确保 mock-mqtt broker 活着
#   4) 写 params.yaml（公钥 PEM）
#   5) 宿主后台起 cart_ota_agent.py（OTA_CONTAINER=roto-builder）
#   6) 容器内起 bridge + ota_app
#   7) mosquitto_pub 发完整 MQTT payload
#   8) 等 result.json（agent docker load/tag/restart/health 完成）
#   9) 验证 result.json → report_last_result → MQTT FINISHED
#
# 用法:
#   bash scripts/ota_run.sh /opt/roto_ws/data/ota/cache/manifest-v1.1.0.json
#   bash scripts/ota_run.sh manifest-v1.2.0.json
# =============================================================================
set -euo pipefail

# ---- 颜色 ----
GREEN='\033[0;32m'; RED='\033[0;31m'; YELLOW='\033[1;33m'; CYAN='\033[0;36m'; NC='\033[0m'
ok()    { echo -e "${GREEN}✅${NC} $*"; }
err()   { echo -e "${RED}❌${NC} $*"; }
warn()  { echo -e "${YELLOW}⚠️${NC} $*"; }
info()  { echo -e "${CYAN}ℹ️${NC} $*"; }

# ---- 预检 ----
[[ $# -ge 1 ]] || { err "缺少 manifest.json 参数"; exit 1; }
MANIFEST="$1"
[[ -f "$MANIFEST" ]] || { err "manifest 不存在: $MANIFEST"; exit 1; }

command -v docker   >/dev/null 2>&1 || { err "docker 未安装"; exit 1; }
command -v jq       >/dev/null 2>&1 || { err "jq 未安装 (apt install -y jq)"; exit 1; }
command -v python3  >/dev/null 2>&1 || { err "python3 未安装"; exit 1; }

OTA_DIR="/opt/roto_ws/data/ota"
CONTAINER="${OTA_CONTAINER:-roto-builder}"
HTTP_PORT="${HTTP_PORT:-18080}"
BROKER_PORT="${BROKER_PORT:-1883}"
CARTRID="${CARTRID:-cart-test}"

info "读取 manifest: $MANIFEST"
VERSION=$(jq -r .version "$MANIFEST")
SHA256=$(jq -r .tarball.sha256 "$MANIFEST")
TARBALL=$(jq -r .tarball.path "$MANIFEST")
SIGNATURE=$(jq -r .tarball.signature_base64 "$MANIFEST")
PUBKEY_PEM=$(jq -r '.tarball.ed25519_pubkey_pem // ""' "$MANIFEST")
MQTT_PAYLOAD=$(jq -c .mqtt_payload "$MANIFEST")

ok "版本: $VERSION"
ok "sha256: ${SHA256:0:16}..."
ok "tarball: $TARBALL ($(ls -lh "$TARBALL" | awk '{print $5}'))"

# ========================================================================
# 1. 确保 mock-mqtt broker 活着
# ========================================================================
if docker ps --format '{{.Names}}' | grep -q '^mock-mqtt$'; then
  ok "mock-mqtt broker 已在运行"
else
  warn "mock-mqtt 不在，尝试 start"
  if docker start mock-mqtt 2>/dev/null; then
    ok "docker start mock-mqtt 成功"
  else
    warn "start 失败，重新 run"
    docker run -d --name mock-mqtt --restart=always \
      -p ${BROKER_PORT}:1883 eclipse-mosquitto:2
  fi
  sleep 1
fi
ss -tlnp | grep -q ":${BROKER_PORT} " || { err "broker port ${BROKER_PORT} 不可达"; exit 1; }
ok "broker port ${BROKER_PORT} OK"

# ========================================================================
# 2. HTTP server (tarball 下载) —— 先 kill 僵尸进程
# ========================================================================
info "清理旧 HTTP server"
pkill -f "http.server ${HTTP_PORT}" 2>/dev/null || true
sleep 1

TARBALL_DIR="$(dirname "$TARBALL")"
cd "$TARBALL_DIR"
python3 -m http.server "${HTTP_PORT}" > /tmp/ota_http.log 2>&1 &
HTTP_PID=$!
cd - > /dev/null
sleep 1

# 验证 server 能下 tarball + sha256 正确
info "验证 HTTP tarball sha256"
HTTP_URL="http://localhost:${HTTP_PORT}/$(basename "$TARBALL")"
REAL_SHA256=$(curl -sL "${HTTP_URL}" | sha256sum | awk '{print $1}')
if [[ "$REAL_SHA256" == "$SHA256" ]]; then
  ok "HTTP sha256 匹配 ✓"
else
  err "HTTP sha256 mismatch! manifest=$SHA256 http=$REAL_SHA256"
  kill "$HTTP_PID" 2>/dev/null; exit 1
fi

# ========================================================================
# 3. 写 params.yaml（公钥 PEM）
# ========================================================================
PARAMS="${OTA_DIR}/params.yaml"
if [[ -n "$PUBKEY_PEM" && "$PUBKEY_PEM" != "null" ]]; then
  info "写 params.yaml"
  cat > "$PARAMS" <<YAMLEOF
app_ota_manager:
  ros__parameters:
    work_dir: ${OTA_DIR}
    download_dir: ${OTA_DIR}/cache
    cart_id: ${CARTRID}
    http_base_url: "http://localhost:${HTTP_PORT}"
    ed25519_pubkey_pem: |
YAMLEOF
  # 把 PEM 每一行缩进 6 空格
  echo "$PUBKEY_PEM" | sed 's/^/      /' >> "$PARAMS"
  ok "params.yaml: $PARAMS"
else
  warn "manifest 里没公钥，写空 params.yaml"
  cat > "$PARAMS" <<YAMLEOF
app_ota_manager:
  ros__parameters:
    work_dir: ${OTA_DIR}
    download_dir: ${OTA_DIR}/cache
    cart_id: ${CARTRID}
    http_base_url: "http://localhost:${HTTP_PORT}"
YAMLEOF
fi

# ========================================================================
# 4. 清旧文件 + 起宿主 agent
# ========================================================================
rm -f "${OTA_DIR}/request.json" "${OTA_DIR}/result.json"
ok "清旧 request.json / result.json"

# kill 旧 agent
pkill -f "cart_ota_agent.py" 2>/dev/null || true
sleep 1

info "起 cart_ota_agent.py (后台)"
OTA_CONTAINER="${CONTAINER}" python3 \
  "$(dirname "$0")/cart_ota_agent.py" -d "${OTA_DIR}" \
  > /tmp/ota_agent.log 2>&1 &
AGENT_PID=$!
sleep 2

# 确认 agent 在跑
if ! kill -0 "$AGENT_PID" 2>/dev/null; then
  err "agent 启动失败，看 /tmp/ota_agent.log"
  cat /tmp/ota_agent.log; exit 1
fi
ok "agent PID=${AGENT_PID}"

# ========================================================================
# 5. 容器内起 bridge + ota_app
# ========================================================================
info "容器内存量节点检查"
docker exec "$CONTAINER" bash -c "source /opt/ros/humble/setup.bash 2>/dev/null; source /ws/install/setup.bash 2>/dev/null; ros2 node list 2>/dev/null || true" | grep -q "comm_mqtt_cloud" && {
  warn "bridge 可能已经在跑，跳过启动"
} || true

# 容器内起 bridge (后台)
info "容器内起 comm_mqtt_cloud"
docker exec "$CONTAINER" bash -c '
source /opt/ros/humble/setup.bash && source /ws/install/setup.bash
pkill -f comm_mqtt_cloud 2>/dev/null || true
sleep 0.5
nohup ros2 run service_cloud_comm comm_mqtt_cloud --ros-args \
  -p cloud_broker:=localhost -p cloud_port:=1883 -p cloud_tls:=false -p device_id:=cart-test \
  > /tmp/bridge.log 2>&1 &
'
sleep 3

# 容器内起 ota_app (前台, 会阻塞) —— 用 nohup 放后台, 但保留日志
info "容器内起 app_ota_manager_node"
docker exec "$CONTAINER" bash -c "
source /opt/ros/humble/setup.bash && source /ws/install/setup.bash
pkill -f app_ota_manager 2>/dev/null || true
sleep 0.5
nohup ros2 run service_ota_app app_ota_manager_node --ros-args --params-file ${PARAMS} \
  > /tmp/ota_app.log 2>&1 &
echo \$!
"
sleep 3

# 验证两个节点都起来了
NODES=$(docker exec "$CONTAINER" bash -c "
source /opt/ros/humble/setup.bash && source /ws/install/setup.bash
ros2 node list 2>/dev/null || true
")
if echo "$NODES" | grep -q "comm_mqtt_cloud" && echo "$NODES" | grep -q "app_ota_manager"; then
  ok "bridge + ota_app 节点就绪 ✓"
else
  err "节点不全: $NODES"
  docker exec "$CONTAINER" cat /tmp/bridge.log 2>/dev/null || true
  docker exec "$CONTAINER" cat /tmp/ota_app.log 2>/dev/null || true
  kill "$AGENT_PID" 2>/dev/null; kill "$HTTP_PID" 2>/dev/null; exit 1
fi

# ========================================================================
# 6. 发 MQTT 指令
# ========================================================================
# 把 payload 里的 image_url 替换成实际 HTTP URL
PAYLOAD=$(echo "$MQTT_PAYLOAD" | jq --arg url "${HTTP_URL}" '.image_url = $url')
PAYLOAD_FILE="/tmp/ota_mqtt_payload.json"
echo "$PAYLOAD" > "$PAYLOAD_FILE"

info "发 MQTT ota/app/version"
if command -v mosquitto_pub >/dev/null 2>&1; then
  mosquitto_pub -h localhost -p "${BROKER_PORT}" \
    -t ota/app/version \
    -f "$PAYLOAD_FILE"
  ok "MQTT 已下发"
else
  warn "mosquitto_pub 没装，用 python paho"
  python3 -c "
import json, paho.mqtt.client as mqtt
p = json.load(open('$PAYLOAD_FILE'))
mqtt.Client(mqtt.CallbackAPIVersion.VERSION2).publish('ota/app/version', json.dumps(p), qos=1) \
  and print('mqtt pub ok') or print('mqtt pub fail')
" 2>/dev/null || {
  warn "python paho 也没装，请手动 publish: $PAYLOAD_FILE"
}
fi

# ========================================================================
# 7. 等 result.json（agent 处理完 request.json → docker load/tag/restart）
# ========================================================================
RESULT_FILE="${OTA_DIR}/result.json"
REQUEST_FILE="${OTA_DIR}/request.json"
echo
info "━━━ 等待 OTA 进度 ━━━"

DEADLINE=$((SECONDS + 240))
while (( SECONDS < DEADLINE )); do
  if [[ -f "$REQUEST_FILE" ]]; then
    echo -n "  $(date +%H:%M:%S) request.json 已生成，agent 处理中...      "
  fi
  if [[ -f "$RESULT_FILE" ]]; then
    echo
    break
  fi
  echo -n "."
  sleep 2
done
echo

if [[ ! -f "$RESULT_FILE" ]]; then
  err "超时 (240s) 没看到 result.json"
  echo
  echo "--- agent log: ---"
  tail -30 /tmp/ota_agent.log
  echo
  echo "--- ota_app log: ---"
  docker exec "$CONTAINER" tail -30 /tmp/ota_app.log 2>/dev/null || true
  echo
  echo "--- bridge log: ---"
  docker exec "$CONTAINER" tail -30 /tmp/bridge.log 2>/dev/null || true
  kill "$AGENT_PID" 2>/dev/null; kill "$HTTP_PID" 2>/dev/null; exit 1
fi

# ========================================================================
# 8. 验证 result.json
# ========================================================================
echo
info "━━━ result.json ━━━"
cat "$RESULT_FILE" | python3 -m json.tool
echo

RES_STATUS=$(jq -r .status "$RESULT_FILE")
RES_VERSION=$(jq -r .target_version "$RESULT_FILE")
RES_TASK=$(jq -r .ota_task_id "$RESULT_FILE")

if [[ "$RES_STATUS" == "success" && "$RES_VERSION" == "$VERSION" ]]; then
  ok "result.json: status=success target_version=$VERSION ✓"
elif [[ "$RES_STATUS" == "rollback_success" ]]; then
  warn "result.json: rollback_success — 新容器健康检查超时，已回滚 previous"
else
  err "result.json: status=$RES_STATUS (message=$(jq -r .message "$RESULT_FILE"))"
fi

# ========================================================================
# 9. report_last_result 最终 MQTT 上报（容器重启后）
# ========================================================================
echo
info "━━━ 检查 report_last_result ━━━"

# 容器重启后之前手动起的 bridge + ota_app 都被 kill 了
# 重新起它们让 report_last_result 执行
CONTAINER_STATE=$(docker inspect -f '{{.State.Running}}' "$CONTAINER" 2>/dev/null || echo "false")
if [[ "$CONTAINER_STATE" == "true" ]]; then
  ok "容器 Running ✓ (restart 后)"
else
  warn "容器不在 Running！(agent 可能失败) 跳过 report"
  kill "$AGENT_PID" 2>/dev/null; kill "$HTTP_PID" 2>/dev/null
  exit 0
fi

# 容器里重起 bridge + ota_app，监听 ros2 topic /ota/status
info "容器内重起 bridge + ota_app (监听 /ota/status)"
docker exec "$CONTAINER" bash -c '
source /opt/ros/humble/setup.bash && source /ws/install/setup.bash

# kill 残留
pkill -f comm_mqtt_cloud 2>/dev/null || true
pkill -f app_ota_manager 2>/dev/null || true
sleep 1

# 起 bridge
nohup ros2 run service_cloud_comm comm_mqtt_cloud --ros-args \
  -p cloud_broker:=localhost -p cloud_port:=1883 -p cloud_tls:=false -p device_id:=cart-test \
  > /tmp/bridge.log 2>&1 &
sleep 2

# 起 ota_app —— 它启动时 report_last_result 会立刻发布 /ota/status
nohup ros2 run service_ota_app app_ota_manager_node --ros-args --params-file '$PARAMS' \
  > /tmp/ota_app.log 2>&1 &
echo "waiting for report_last_result..."

# 监听 ROS /ota/status 最多 15s
timeout 15 ros2 topic echo --once /ota/status 2>/dev/null || echo "timeout waiting /ota/status"
'

# 给点时间让 bridge 编码 → MQTT
sleep 2

# 看 broker 上 ota/status 最后一条消息
if command -v mosquitto_sub >/dev/null 2>&1; then
  echo
  info "━━━ MQTT ota/status (最后一条) ━━━"
  timeout 3 mosquitto_sub -h localhost -p "${BROKER_PORT}" \
    -t ota/status -C 1 -R 2>/dev/null || echo "(没有消息或 broker 空)"
fi

# ========================================================================
# 10. 收尾
# ========================================================================
echo
info "━━━ 清理 ━━━"
kill "$AGENT_PID" 2>/dev/null && ok "agent 已停 (PID=$AGENT_PID)" || info "agent 已停"
kill "$HTTP_PID"  2>/dev/null && ok "HTTP server 已停 (PID=$HTTP_PID)" || info "HTTP server 已停"

# 容器内保留 bridge + ota_app 继续跑，让它上报后续 heartbeat
echo
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
if [[ "$RES_STATUS" == "success" && "$RES_VERSION" == "$VERSION" ]]; then
  ok "🎉 OTA 闭环完成！version=$VERSION task=$RES_TASK"
else
  warn "⚠️  OTA 完成但 status=$RES_STATUS"
fi
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
