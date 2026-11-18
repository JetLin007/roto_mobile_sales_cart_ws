#!/usr/bin/env bash
# ==============================================================================
# 文件名：scripts/check_runtime_env.sh
# 用途：核对 RK3576S 出厂 Ubuntu 22.04 的 L0 运行时环境（架构 v1.2 §4.2）
# 用法：sudo ./scripts/check_runtime_env.sh                 # 只查宿主
#       sudo ./scripts/check_runtime_env.sh --image <img>   # 追加镜像内 ROS2/DDS 校验
#       sudo ./scripts/check_runtime_env.sh -v              # 打印每条原始输出
# 判定口径：宿主必须有 docker/systemd/chrony/内核共享内存与多播；
#          ROS2 Humble 与 CycloneDDS 属容器镜像内组件，宿主缺失是正常状态。
# ==============================================================================
set -uo pipefail

IMAGE=""
VERBOSE=0
while [ $# -gt 0 ]; do
  case "$1" in
    --image) IMAGE="${2:-}"; shift 2 ;;
    -v|--verbose) VERBOSE=1; shift ;;
    -h|--help) sed -n '3,9p' "$0"; exit 0 ;;
    *) echo "未知参数: $1" >&2; exit 2 ;;
  esac
done

PASS=0; WARN=0; FAIL=0; SKIP=0
ok()   { PASS=$((PASS+1)); printf '  [ OK ] %s\n' "$*"; }
warn() { WARN=$((WARN+1)); printf '  [WARN] %s\n' "$*"; }
bad()  { FAIL=$((FAIL+1)); printf '  [FAIL] %s\n' "$*"; }
skip() { SKIP=$((SKIP+1)); printf '  [SKIP] %s\n' "$*"; }
sec()  { printf '\n== %s ==\n' "$*"; }
run()  { if [ "$VERBOSE" -eq 1 ]; then printf '  $ %s\n' "$*"; bash -c "$*" 2>&1 | sed 's/^/      /'; fi; }

# ------------------------------------------------------------------
# 0. 宿主基础信息 + 架构 + 发行版
# ------------------------------------------------------------------
sec "0. 宿主基础信息"
printf '  主机名   : %s\n' "$(hostname)"
printf '  内核     : %s\n' "$(uname -r)"
printf '  架构     : %s\n' "$(uname -m)"
printf '  发行版   : %s\n' "$(. /etc/os-release 2>/dev/null && echo "$PRETTY_NAME")"
printf '  运行时长 : %s\n' "$(uptime -p 2>/dev/null || echo n/a)"
run "uname -a"; run "cat /etc/os-release"

ARCH=$(uname -m)
case "$ARCH" in
  aarch64|arm64) ok "架构 $ARCH（RK3576S 预期）" ;;
  *)             bad "架构 $ARCH 不是 aarch64，确认是否在板子上执行" ;;
esac

VER=$(. /etc/os-release && echo "$VERSION_ID")
if [ "$VER" = "22.04" ]; then ok "Ubuntu $VER（Humble 对应基线）"; else bad "Ubuntu $VER != 22.04"; fi
TZ_NOW=$(timedatectl show -p Timezone --value 2>/dev/null)
[ -n "${TZ_NOW:-}" ] || TZ_NOW=$(cat /etc/timezone 2>/dev/null || readlink -f /etc/localtime 2>/dev/null | sed 's|.*zoneinfo/||')
TZ_NOW=${TZ_NOW:-unknown}
printf '  当前时区 : %s\n' "$TZ_NOW"

# ------------------------------------------------------------------
# 1. systemd：必须是 PID 1，否则 cart-watchdog / mosquitto / OTA 全失效
# ------------------------------------------------------------------
sec "1. systemd（宿主服务管理）"
PID1=$(ps -p 1 -o comm= 2>/dev/null | tr -d ' ')
if [ "$PID1" = "systemd" ]; then
  SYSDV=$(systemctl --version 2>/dev/null | awk 'NR==1{print $2}')
  ok "PID 1 = systemd，版本 ${SYSDV:-未知}"
else
  bad "PID 1 = ${PID1:-未知}，不是 systemd"
fi
if [ -d /run/systemd/system ]; then ok "/run/systemd/system 存在（服务可被 systemd 托管）"; else warn "缺少 /run/systemd/system"; fi
run "systemctl --version | head -1"

# ------------------------------------------------------------------
# 2. Docker：出厂已带，但要确认 daemon 真在跑、随系统自启
# ------------------------------------------------------------------
sec "2. Docker（容器运行时）"
if command -v docker >/dev/null 2>&1; then
  DV=$(docker version --format '{{.Server.Version}}' 2>/dev/null)
  if [ -n "$DV" ]; then ok "docker 客户端+daemon 可用，Server $DV"; else bad "docker 客户端在但 daemon 不可达（systemctl status docker）"; fi
  if systemctl is-enabled --quiet docker 2>/dev/null; then ok "docker.service 已 enabled（开机自启）"; else warn "docker.service 未 enabled，重启后容器不自启"; fi
  if systemctl is-active --quiet docker 2>/dev/null; then ok "docker.service active"; else bad "docker.service 未 active"; fi
  printf '  Storage  : %s\n' "$(docker info --format '{{.Driver}}' 2>/dev/null)"
  printf '  Cgroup   : %s\n' "$(docker info --format '{{.CgroupDriver}}/{{.CgroupVersion}}' 2>/dev/null)"
  printf '  数据目录 : %s\n' "$(docker info --format '{{.DockerRootDir}}' 2>/dev/null)"
  run "docker version --format '{{.Server.Version}}'"
  run "docker info --format '{{.Driver}} {{.CgroupDriver}} {{.DockerRootDir}}'"
else
  bad "未安装 docker（与“出厂默认带 docker”的前提不符）"
fi

# ------------------------------------------------------------------
# 3. 宿主 systemd 服务清单（§8.3）：docker 必须已有，cart-* 属待部署项
# ------------------------------------------------------------------
sec "3. 宿主 systemd 服务清单（架构 §8.3）"
declare -A MUST=([docker]=must [chrony]=must)
declare -A OPT=([cart-ota-agent]=deploy [cart-watchdog]=deploy [cart-network-init]=deploy [mosquitto]=deploy)
for u in docker chrony; do
  if systemctl cat "${u}.service" >/dev/null 2>&1 || systemctl list-unit-files "${u}.service" 2>/dev/null | grep -q "${u}.service"; then
    st=$(systemctl is-active "${u}.service" 2>/dev/null)
    ok "已安装 ${u}.service（state=${st}）"
  else
    bad "缺失 ${u}.service（${MUST[$u]}）"
  fi
done
for u in cart-ota-agent cart-watchdog cart-network-init mosquitto; do
  if systemctl cat "${u}.service" >/dev/null 2>&1; then
    ok "已安装 ${u}.service（$(systemctl is-active "${u}.service" 2>/dev/null)）"
  else
    skip "${u}.service 未安装（步骤 7 交付，属预期）"
  fi
done

# ------------------------------------------------------------------
# 4. chrony：宿主唯一时基收敛出口（架构 v1.2 P10）
# ------------------------------------------------------------------
sec "4. chrony（宿主时基，GNSS/云对时的收敛出口）"
if command -v chronyd >/dev/null 2>&1; then
  ok "chronyd 已安装：$(chronyd -v 2>/dev/null | head -1)"
else
  bad "未安装 chrony（apt install chrony）"
fi
if systemctl is-active --quiet chrony 2>/dev/null; then ok "chrony.service active"; else warn "chrony.service 未运行"; fi
if command -v chronyc >/dev/null 2>&1; then
  LEAP=$(chronyc tracking 2>/dev/null | awk -F': *' '/Leap status/{print $2}')
  STRATUM=$(chronyc tracking 2>/dev/null | awk -F': *' '/Stratum/{print $2}')
  OFFSET=$(chronyc tracking 2>/dev/null | awk -F': *' '/System time/{print $2}')
  printf '  Leap=%s Stratum=%s Offset=%s\n' "${LEAP:-?}" "${STRATUM:-?}" "${OFFSET:-?}"
  if [ "${LEAP:-}" = "Normal" ]; then ok "chrony 状态正常（Leap=Normal）"; else warn "chrony 状态异常（Leap=${LEAP:-未知}）"; fi
  if [ "${STRATUM:-0}" = "0" ] 2>/dev/null; then warn "Stratum=0，当前无可用时间源（云对时代理未接通属正常）"; fi
  SRCN=$(chronyc sources 2>/dev/null | grep -c '^\^[*?]' || true)
  printf '  可用时间源数量 : %s\n' "${SRCN:-0}"
  run "chronyc tracking"
  run "chronyc sources -v"
fi
NTP_SYNC=$(timedatectl show -p NTPSynchronized --value 2>/dev/null || echo unknown)
printf '  timedatectl NTPSynchronized = %s\n' "$NTP_SYNC"
if [ "$NTP_SYNC" = "yes" ]; then ok "系统时钟已同步"; else warn "系统时钟未同步（首次上电/无网时正常，云对时代理接入后应变 yes）"; fi

# ------------------------------------------------------------------
# 5. GNSS / PPS（可选源）：v1.2 P10 已降级为非必需
# ------------------------------------------------------------------
sec "5. GNSS / PPS（可选时源，非必需）"
SERIALS=$(ls /dev/ttyS* /dev/ttyUSB* /dev/ttyACM* 2>/dev/null | tr '\n' ' ')
printf '  串口设备 : %s\n' "${SERIALS:-无}"
PPS_N=$(ls -A /sys/class/pps 2>/dev/null | grep -c . || true)
if ls /dev/pps* >/dev/null 2>&1 || [ "${PPS_N:-0}" -gt 0 ]; then
  ok "存在 PPS 设备/子系统：$(ls /dev/pps* /sys/class/pps 2>/dev/null | tr '\n' ' ')"
else
  skip "无 PPS 设备（架构 v1.2 以云端四时间戳为主时源，可接受）"
fi
if dmesg 2>/dev/null | grep -qiE 'gnss|u-blox|gps'; then ok "内核日志出现 GNSS/GPS 关键字"; else skip "内核日志无 GNSS 关键字"; fi
run "dmesg | grep -iE 'gnss|u-blox|gps' | tail -5"
run "ls -l /dev/ttyS* /dev/ttyUSB* 2>/dev/null"

# ------------------------------------------------------------------
# 6. DDS 依赖的内核能力：/dev/shm 与多播（CycloneDDS 的隐藏前置）
# ------------------------------------------------------------------
sec "6. DDS 依赖的内核能力（CycloneDDS 前置）"
SHM_TOTAL=$(df -h /dev/shm 2>/dev/null | awk 'NR==2{print $2}')
SHM_USE=$(df -h /dev/shm 2>/dev/null | awk 'NR==2{print $5}' | tr -d '%')
printf '  /dev/shm : 总 %s，已用 %s%%\n' "${SHM_TOTAL:-?}" "${SHM_USE:-?}"
SHM_MB=$(df -m /dev/shm 2>/dev/null | awk 'NR==2{print $2}')
if [ -n "${SHM_MB:-}" ] && [ "$SHM_MB" -ge 256 ]; then
  ok "/dev/shm 容量 ${SHM_MB}MB，满足单机 DDS 共享内存"
else
  warn "/dev/shm 仅 ${SHM_MB:-?}MB，容器默认 64MB 会导致 CycloneDDS 共享内存失败，需 --ipc=host 或 --shm-size=512m"
fi
printf '  kernel.shmmax : %s\n' "$(sysctl -n kernel.shmmax 2>/dev/null)"
MC=$(ip -o link show 2>/dev/null | grep -c MULTICAST)
if [ "${MC:-0}" -gt 0 ]; then ok "${MC} 个网卡支持 MULTICAST（DDS 发现需要）"; else warn "无 MULTICAST 网卡，DDS 发现会退化"; fi
run "ip -o link show | grep MULTICAST | head -5"
run "sysctl kernel.shmmax kernel.shmall"

# ------------------------------------------------------------------
# 7. 宿主 ROS2：架构要求“固化于容器镜像”，宿主存在反而是风险
# ------------------------------------------------------------------
sec "7. 宿主 ROS2（预期不存在）"
if [ -d /opt/ros ]; then
  warn "宿主存在 /opt/ros：$(ls /opt/ros 2>/dev/null | tr '\n' ' ')，与“固化容器”冲突，存在版本漂移风险"
else
  ok "宿主无 /opt/ros（符合架构 §4.2）"
fi
if command -v ros2 >/dev/null 2>&1; then warn "宿主 PATH 里有 ros2：$(command -v ros2)"; else ok "宿主 PATH 无 ros2"; fi
printf '  RMW_IMPLEMENTATION=%s\n' "${RMW_IMPLEMENTATION:-<未设置>}"

# ------------------------------------------------------------------
# 8. 容器镜像内 ROS2 / CycloneDDS（真正跑应用的地方）
# ------------------------------------------------------------------
sec "8. 容器镜像内 ROS2 / CycloneDDS"
if [ -z "$IMAGE" ]; then
  printf '  本地镜像列表：\n'
  docker images --format '      {{.Repository}}:{{.Tag}}  {{.Size}}' 2>/dev/null | head -10
  skip "未指定 --image，跳过镜像内校验（加 --image <仓库:tag> 再跑一次）"
else
  if docker image inspect "$IMAGE" >/dev/null 2>&1; then
    ok "镜像存在：$IMAGE"
    docker run --rm --net=host --ipc=host "$IMAGE" bash -lc '
      echo "  ROS_DISTRO=${ROS_DISTRO:-未设置}"
      echo "  RMW_IMPLEMENTATION=${RMW_IMPLEMENTATION:-未设置}"
      if [ -f /opt/ros/humble/setup.bash ]; then echo "  [ OK ] /opt/ros/humble/setup.bash"; else echo "  [FAIL] 无 /opt/ros/humble/setup.bash"; fi
      if [ -f /opt/ros/humble/lib/librmw_cyclonedds_cpp.so ]; then echo "  [ OK ] librmw_cyclonedds_cpp.so"; else echo "  [FAIL] 无 CycloneDDS RMW 库"; fi
      command -v ros2 >/dev/null 2>&1 && echo "  [ OK ] $(ros2 --version 2>&1 | head -1)" || echo "  [WARN] ros2 命令不在 PATH"
      ls /opt/ros/humble/lib/librosidl_typesupport_cpp.so >/dev/null 2>&1 && echo "  [ OK ] typesupport 就绪"
    ' 2>&1 | sed 's/^/  /'
  else
    bad "镜像不存在：$IMAGE（先 docker load/pull）"
  fi
fi

# ------------------------------------------------------------------
# 汇总
# ------------------------------------------------------------------
sec "汇总"
printf '  PASS=%d  WARN=%d  FAIL=%d  SKIP=%d\n' "$PASS" "$WARN" "$FAIL" "$SKIP"
if [ "$FAIL" -eq 0 ]; then
  printf '  结论：宿主运行时基线可用；容器内 ROS2/DDS 以第 8 项结果为准。\n'
else
  printf '  结论：存在 %d 项硬缺失，先修 FAIL 项再进入步骤 2。\n' "$FAIL"
fi
exit "$([ "$FAIL" -eq 0 ] && echo 0 || echo 1)"
