#!/usr/bin/env bash
# =============================================================================
# ota_verify.sh — OTA 升级后一键验证（容器重启后）
# -----------------------------------------------------------------------------
# 用法:
#   # 宿主上一键跑（自动 docker exec 进容器 + 宿主侧 docker 检查）
#   bash scripts/ota_verify.sh
#
#   # 指定版本 + 容器名
#   bash scripts/ota_verify.sh --target 1.1.0 --container roto-builder
#
#   # 已经在容器内了（只做容器内检查）
#   bash /ws/scripts/ota_verify.sh --self
# =============================================================================
set -eo pipefail

GREEN='\033[0;32m'; RED='\033[0;31m'; YELLOW='\033[1;33m'; CYAN='\033[0;36m'; NC='\033[0m'
ok()    { echo -e "  ${GREEN}✓${NC} $*"; }
err()   { echo -e "  ${RED}✗${NC} $*"; }
warn()  { echo -e "  ${YELLOW}!${NC} $*"; }
info()  { echo -e "  ${CYAN}→${NC} $*"; }

OTA_DIR="/opt/roto_ws/data/ota"
CONTAINER="roto-builder"
TARGET_VERSION=""
SELF_MODE=0

# ---- 解析参数 ----
while [[ $# -gt 0 ]]; do
  case "$1" in
    -c|--container) CONTAINER="$2"; shift 2 ;;
    -v|--target)    TARGET_VERSION="$2"; shift 2 ;;
    --self)         SELF_MODE=1; shift ;;
    *) echo "未知参数: $1"; exit 1 ;;
  esac
done

# ---- 判断是否在容器内 ----
if [[ -f /.dockerenv ]] && [[ -n "${ROS_DISTRO:-}" ]]; then
  SELF_MODE=1
fi

# ---- 宿主侧: 先跑 docker 检查，再 exec 进容器 ----
if [[ "$SELF_MODE" -eq 0 ]] && command -v docker >/dev/null 2>&1; then
  echo -e "${CYAN}━━━ OTA 宿主侧 Docker 检查 ━━━${NC}\n"

  PASS=0; FAIL=0
  pass() { ok "$1"; PASS=$((PASS+1)); }
  fail() { err "$1"; FAIL=$((FAIL+1)); }

  info "1. 容器状态"
  if docker ps --format '{{.Names}}' | grep -q "^${CONTAINER}$"; then
    pass "${CONTAINER} 在 Running"
    UP_SEC=$(docker inspect -f '{{.State.StartedAt}}' "${CONTAINER}" 2>/dev/null \
      | xargs -I{} date -d {} +%s 2>/dev/null || echo "0")
    NOW_SEC=$(date +%s)
    if [[ "$UP_SEC" != "0" ]]; then
      RUN_MIN=$(( (NOW_SEC - UP_SEC) / 60 ))
      echo "    容器已运行 ${RUN_MIN} 分钟 (started=$(docker inspect -f '{{.State.StartedAt}}' ${CONTAINER}))"
    fi
    RESTARTS=$(docker inspect -f '{{.RestartCount}}' "${CONTAINER}" 2>/dev/null || echo "?")
    echo "    RestartCount=${RESTARTS}"
  else
    fail "${CONTAINER} 不在 Running"
    docker ps -a --filter name="${CONTAINER}" --format '{{.Status}}' 2>/dev/null || true
  fi

  echo
  info "2. roto-cart 镜像标签"
  TAG_COUNT=0
  for tag in current previous; do
    if docker image inspect "roto-cart:${tag}" >/dev/null 2>&1; then
      pass "roto-cart:${tag} 存在"
      TAG_COUNT=$((TAG_COUNT+1))
    else
      warn "roto-cart:${tag} 不存在"
    fi
  done
  # 所有带版本号的 roto-cart:*
  VERSION_TAGS=$(docker images --format '{{.Tag}}' | grep '^roto-cart:' | grep -v -E 'current|previous' || true)
  if [[ -n "$VERSION_TAGS" ]]; then
    echo "    版本标签:"
    echo "$VERSION_TAGS" | sed 's/^/      roto-cart:/'
    TAG_COUNT=$((TAG_COUNT + $(echo "$VERSION_TAGS" | wc -l)))
    pass "共 ${TAG_COUNT} 个 roto-cart 标签 (含版本号)"
  else
    warn "没有带版本号的镜像标签"
  fi

  # 如果指定了 --target, 检查该版本 OCI label
  if [[ -n "$TARGET_VERSION" ]]; then
    echo
    info "3. 指定版本 OCI label (target=${TARGET_VERSION})"
    IMG="roto-cart:${TARGET_VERSION}"
    if docker image inspect "$IMG" >/dev/null 2>&1; then
      REV=$(docker inspect -f '{{index .Config.Labels "org.opencontainers.image.revision"}}' "$IMG" 2>/dev/null)
      VER=$(docker inspect -f '{{index .Config.Labels "org.opencontainers.image.version"}}' "$IMG" 2>/dev/null)
      echo "    revision=${REV:-<空>}"
      echo "    version=${VER:-<空>}"
      [[ -n "$REV" && "$REV" != "<no value>" ]] && pass "OCI revision label 非空 (真实 commit hash)" || warn "OCI revision label 缺失"
      [[ "$VER" == "$TARGET_VERSION" ]] && pass "OCI version label = ${TARGET_VERSION}" || warn "OCI version label 不匹配"
    else
      err "${IMG} 镜像不存在"
    fi
  fi

  echo
  info "4. mock-mqtt broker"
  if docker ps --format '{{.Names}}' | grep -q '^mock-mqtt$'; then
    pass "mock-mqtt broker 运行中"
    BROKER_PID=$(docker inspect -f '{{.State.Pid}}' mock-mqtt 2>/dev/null || echo "0")
    echo "    broker container PID=${BROKER_PID}"
  else
    fail "mock-mqtt 不在运行"
  fi

  echo
  echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
  echo -e "  宿主侧: ${GREEN}${PASS} PASSED${NC}  ${RED}${FAIL} FAILED${NC}"
  echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
  echo

  if [[ "$FAIL" -gt 0 ]]; then
    err "宿主侧有检查失败，不继续进容器"
    exit 1
  fi

  # ---- 进容器跑内部检查 ----
  echo -e "${CYAN}━━━ 进入容器 ${CONTAINER} 检查 ━━━${NC}\n"
  docker exec -e TARGET_VERSION="$TARGET_VERSION" -e OTA_DIR="$OTA_DIR" "$CONTAINER" \
    bash -c "SELF_MODE=1 TARGET_VERSION='${TARGET_VERSION}' OTA_DIR='${OTA_DIR}' bash /ws/scripts/ota_verify.sh --self" \
    && exit 0 || exit $?
fi

# ============================================================================
# 以下是容器内检查 (SELF_MODE=1 或原生在容器内)
# ============================================================================

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  OTA 容器内验证"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

PASS=0; FAIL=0
pass() { ok "$1"; PASS=$((PASS+1)); }
fail() { err "$1"; FAIL=$((FAIL+1)); }

# ---- 0. 加载 ROS 环境 ----
echo
info "0. ROS + workspace 环境"
source /opt/ros/humble/setup.bash 2>/dev/null && pass "source /opt/ros/humble/setup.bash" || fail "ROS Humble 不存在"
if [[ -f /ws/install/setup.bash ]]; then
  source /ws/install/setup.bash 2>/dev/null
  pass "source /ws/install/setup.bash"
else
  fail "workspace /ws/install 不存在 (没 colcon build?)"
fi

# ============================================================================
# 1. 基础环境
# ============================================================================
echo
info "1. 基础环境"

[[ -d "$OTA_DIR" ]] && pass "/opt/roto_ws/data/ota 共享目录存在" || fail "OTA 共享目录不存在"
[[ -d "${OTA_DIR}/cache" ]] && pass "cache 目录存在" || fail "cache 目录不存在"
command -v ros2 >/dev/null 2>&1 && pass "ros2 CLI 可用" || fail "ros2 CLI 不可用"

PKG_PREFIX=$(ros2 pkg prefix service_ota_app 2>/dev/null || echo "")
[[ -n "$PKG_PREFIX" && -d "$PKG_PREFIX" ]] \
  && pass "service_ota_app 包已安装 ($PKG_PREFIX)" \
  || fail "service_ota_app 包不存在"

# cache 可写测试
touch "${OTA_DIR}/cache/.write_test_$$" 2>/dev/null && rm "${OTA_DIR}/cache/.write_test_$$" \
  && pass "cache 目录可写" || fail "cache 目录不可写"

# ============================================================================
# 2. result.json (agent 写入, OTA 核心证据)
# ============================================================================
echo
info "2. result.json (宿主 cart_ota_agent.py 写入)"

RESULT_FILE="${OTA_DIR}/result.json"

if [[ -f "$RESULT_FILE" ]]; then
  pass "result.json 存在"

  python3 -c "
import json
d = json.load(open('$RESULT_FILE'))
print()
print('    ┌─ result.json 内容 ──────────────────────────')
for k in ['ota_task_id','target_version','status','message','image_ref','timestamp_ms']:
    print(f'    │ {k:20s} = {d.get(k,\"\")}')
print('    └─────────────────────────────────────────────')
"

  RES_STATUS=$(python3 -c "import json; print(json.load(open('$RESULT_FILE')).get('status',''))")
  RES_VERSION=$(python3 -c "import json; print(json.load(open('$RESULT_FILE')).get('target_version',''))")
  RES_TASK=$(python3 -c "import json; print(json.load(open('$RESULT_FILE')).get('ota_task_id',''))")
  RES_TS=$(python3 -c "import json; print(json.load(open('$RESULT_FILE')).get('timestamp_ms',0))")

  # status 检查
  case "$RES_STATUS" in
    success)          pass "status = success ✓" ;;
    rollback_success) warn "status = rollback_success (回滚成功, 说明新容器启动失败过)" ; PASS=$((PASS+1)) ;;
    rollback_failed)  fail "status = rollback_failed (回滚也失败了!)" ;;
    *)                fail "status = ${RES_STATUS:-<空>} (期望 success)" ;;
  esac

  # target_version 检查
  [[ -n "$RES_VERSION" ]] && pass "target_version = ${RES_VERSION}" || fail "target_version 为空"
  if [[ -n "$TARGET_VERSION" && "$RES_VERSION" == "$TARGET_VERSION" ]]; then
    pass "target_version 匹配期望 (${TARGET_VERSION}) ✓"
  elif [[ -n "$TARGET_VERSION" ]]; then
    fail "target_version 不匹配: result=${RES_VERSION} expected=${TARGET_VERSION}"
  fi

  [[ -n "$RES_TASK" ]] && pass "ota_task_id = ${RES_TASK}" || fail "ota_task_id 为空"

  # 时间戳合理性 (在最近 2 小时内)
  if [[ "$RES_TS" -gt 0 ]]; then
    NOW_MS=$(date +%s000)
    DIFF_MIN=$(( (NOW_MS - RES_TS) / 60000 ))
    if [[ "$DIFF_MIN" -ge 0 && "$DIFF_MIN" -lt 720 ]]; then
      pass "timestamp 合理 (距今 ${DIFF_MIN} 分钟)"
    elif [[ "$DIFF_MIN" -lt 0 ]]; then
      warn "timestamp 在未来 (时钟漂移?) 差 $((-DIFF_MIN)) 分钟"
    else
      warn "timestamp 太老 (${DIFF_MIN} 分钟前), 可能是之前 OTA 残留"
    fi
  fi
else
  fail "result.json 不存在 (agent 还没执行?)"
fi

# ============================================================================
# 3. params.yaml
# ============================================================================
echo
info "3. params.yaml (ota_app 配置)"

PARAMS="${OTA_DIR}/params.yaml"
if [[ -f "$PARAMS" ]]; then
  pass "params.yaml 存在"
  head -15 "$PARAMS" | sed 's/^/    /'
  echo "    ..."
  grep -q "^app_ota_manager:" "$PARAMS" \
    && pass "顶层 key = app_ota_manager" \
    || fail "顶层 key 不是 app_ota_manager (常见错误: app_ota_manager_node)"
  grep -q "work_dir:" "$PARAMS" && pass "含 work_dir" || warn "缺 work_dir (用默认值)"
  grep -q "ed25519_pubkey_pem:" "$PARAMS" && pass "含 ed25519_pubkey_pem" || warn "缺公钥 (签名校验跳过)"
else
  warn "params.yaml 不存在 (ota_app 会用默认 work_dir=${OTA_DIR})"
fi

# ============================================================================
# 4. tarball cache
# ============================================================================
echo
info "4. tarball cache"
if ls "${OTA_DIR}/cache/"*.tar.gz >/dev/null 2>&1; then
  ls -lh "${OTA_DIR}/cache/"*.tar.gz | sed 's/^/    /'
  pass "tarball 存在 ✓"
else
  warn "cache 里没有 .tar.gz"
fi

# ============================================================================
# 5. report_last_result → ROS /ota/status → MQTT FINISHED
# ============================================================================
echo
info "5. report_last_result → ROS → MQTT 最终上报"

# 清理残留
pkill -f comm_mqtt_cloud 2>/dev/null || true
pkill -f app_ota_manager 2>/dev/null || true
sleep 1

# 起 bridge (后台)
ros2 run service_cloud_comm comm_mqtt_cloud --ros-args \
  -p cloud_broker:=localhost -p cloud_port:=1883 -p cloud_tls:=false -p device_id:=cart-test \
  > /tmp/verify_bridge.log 2>&1 &
BRIDGE_PID=$!
sleep 2

# 起 ota_app (后台, 启动瞬间 report_last_result)
[[ -f "$PARAMS" ]] && PARAM_ARG="--params-file $PARAMS" || PARAM_ARG=""
ros2 run service_ota_app app_ota_manager_node --ros-args $PARAM_ARG \
  > /tmp/verify_ota.log 2>&1 &
OTA_PID=$!
sleep 4

# 5a. 节点检查
NODES=$(ros2 node list 2>/dev/null || true)
echo "    节点列表:"
echo "$NODES" | sed 's/^/      /'
echo
echo "$NODES" | grep -q "comm_mqtt_cloud" && pass "bridge /comm_mqtt_cloud 存在" || fail "bridge 节点不存在"
echo "$NODES" | grep -q "app_ota_manager" && pass "ota_app /app_ota_manager 存在" || fail "ota_app 节点不存在"

# 5b. report_last_result 日志
OTA_LOG="/tmp/verify_ota.log"
if grep -q "report last OTA result" "$OTA_LOG" 2>/dev/null; then
  pass "report_last_result 被调用 ✓"
  echo "    report_last_result 日志:"
  grep "report last OTA result" "$OTA_LOG" | sed 's/^/      /'
else
  fail "没看到 report_last_result"
  echo "    完整 ota_app log:"
  cat "$OTA_LOG" 2>/dev/null | sed 's/^/      /' | tail -20
fi

# 5c. FINISHED + SUCCESS
if grep -Eq 'stage=.?4.*progress=.?100.*result=.?1' "$OTA_LOG" 2>/dev/null; then
  pass "stage=4 progress=100 result=1 (FINISHED + SUCCESS) ✓"
  echo "    最终状态:"
  grep -E 'stage=4.*progress=100' "$OTA_LOG" | sed 's/^/      /' | tail -3
else
  fail "没看到 FINISHED+SUCCESS"
  echo "    相关日志:"
  grep -E 'stage=|progress=|result=' "$OTA_LOG" 2>/dev/null | sed 's/^/      /' | tail -10
fi

# 5d. bridge MQTT 连接状态
BRIDGE_LOG="/tmp/verify_bridge.log"
if grep -q "Connected to cloud MQTT" "$BRIDGE_LOG" 2>/dev/null; then
  pass "bridge 已连接 MQTT broker"
else
  warn "bridge 没连上 broker"
  grep -q "subscribe failed" "$BRIDGE_LOG" 2>/dev/null && \
    echo "    (subscribe failed - broker 可能没在跑)"
fi

# 5e. /ota/command 订阅 (bridge 在监听 MQTT 指令)
if ros2 topic info /ota/command 2>/dev/null | grep -q "Subscription"; then
  pass "/ota/command 有订阅 (bridge 正常)"
else
  warn "/ota/command 没有订阅 (bridge 可能没完全连上)"
fi

# ============================================================================
# 6. 清理 + 汇总
# ============================================================================
kill "$BRIDGE_PID" 2>/dev/null || true
kill "$OTA_PID" 2>/dev/null   || true

echo
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "  结果: ${GREEN}${PASS} PASSED${NC}  ${RED}${FAIL} FAILED${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

[[ "$FAIL" -eq 0 ]] && echo -e "\n  ${GREEN}🎉 OTA 验证通过！容器重启后状态正确${NC}" || \
  echo -e "\n  ${RED}⚠️  有 ${FAIL} 项检查未通过，需要排查${NC}"

exit "$FAIL"
