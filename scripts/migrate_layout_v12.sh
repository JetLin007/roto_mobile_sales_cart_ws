#!/usr/bin/env bash
# ==============================================================================
# 文件名：scripts/migrate_layout_v12.sh
# 用途：把存量仓库 src/ 迁移到架构 v1.2 §7 结构
#      本脚本只做**机械动作**：目录改名 + package.xml/CMake 名称同步 + 新建缺失包空骨架
#      **不做**：包合并、包拆分、业务代码搬迁（这些必须人工看代码后逐步做）
# 用法：
#   ./scripts/migrate_layout_v12.sh --dry-run     # 只看会做什么，不动文件（默认）
#   ./scripts/migrate_layout_v12.sh --apply       # 真正执行
# 前置：
#   1) 已在 refactor/v1.2-arch 分支
#   2) 已打基线 tag：git tag pre-v1.2-arch && git push --tags
# ==============================================================================
set -uo pipefail

MODE="dry"
case "${1:-}" in
  --apply) MODE="apply" ;;
  --dry-run|"") MODE="dry" ;;
  -h|--help) sed -n '3,12p' "$0"; exit 0 ;;
  *) echo "未知参数：$1" >&2; exit 2 ;;
esac

cd "${SRC_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}" || exit 1
[ -d src ] || { echo "[ERR] 当前目录无 src/：$(pwd)" >&2; exit 1; }
[ -d .git ] || { echo "[ERR] 不是 git 仓库，先 git init 或用仓库根执行" >&2; exit 1; }

BRANCH=$(git rev-parse --abbrev-ref HEAD)
echo "== 迁移计划（模式：$MODE）=="
echo "  仓库   : $(pwd)"
echo "  分支   : $BRANCH"
if ! git tag -l | grep -q '^pre-v1.2-arch$'; then
  echo "  [WARN] 缺基线 tag，强烈建议先执行：git tag pre-v1.2-arch"
fi
echo

run () {  # run "描述" 命令...
  local desc=$1; shift
  if [ "$MODE" = "apply" ]; then
    echo "  [RUN ] $desc"
    "$@" || { echo "  [FAIL] $desc" >&2; exit 1; }
  else
    echo "  [DRY ] $desc"
  fi
}

# ---------- 1) 纯改名：<旧路径> <新路径> <新包名或 -> ----------
# 说明：app_* 目录名带 _node 后缀而 package.xml 内 <name> 已去掉后缀，改名时以 <name> 为准
RENAMES=(
  "src/app_layer/app_alarm_warn_node      src/app_alarm_warn      -"
  "src/app_layer/app_order_task_node      src/app_order_task      -"
  "src/app_layer/app_remote_video_node    src/app_remote_video    -"
  "src/app_layer/ota_manager_node         src/service_ota_app     service_ota_app"
  "src/comm_layer/comm_mqtt_cloud         src/service_cloud_comm  service_cloud_comm"
  "src/comm_layer/comm_mqtt_private       src/service_local_comm  service_local_comm"
  # 无歧义的驱动（名字与 §7 完全一致，只是当前埋在 driver_layer 下）
  "src/driver_layer/driver_camera        src/driver_camera       -"
  "src/driver_layer/driver_gps           src/driver_gps          -"
  "src/driver_layer/driver_ultrasonic    src/driver_ultrasonic   -"
)
for row in "${RENAMES[@]}"; do
  read -r old new name <<<"$row"
  [ -d "$old" ] || { echo "  [SKIP] $old 不存在（可能已迁移）"; continue; }
  if [ -e "$new" ]; then echo "  [SKIP] 目标已存在：$new"; continue; fi
  run "git mv $old -> $new" git mv "$old" "$new"
  if [ "$name" != "-" ]; then
    run "  改包名 $new/package.xml: <name>→$name" \
      sed -i "s|<name>[^<]*</name>|<name>$name</name>|" "$new/package.xml"
    run "  改工程名 $new/CMakeLists.txt: project()→$name" \
      sed -i "s|project([A-Za-z0-9_]*)|project($name)|" "$new/CMakeLists.txt"
  fi
done

# ---------- 2) 移出 src：模拟工具不参与发布构建 ----------
if [ -d src/sim_tools ]; then
  mkdir -p tools
  run "git mv src/sim_tools -> tools/sim（§7 无此目录，移出 src 避免进发布镜像）" \
    git mv src/sim_tools tools/sim
fi

# ---------- 3) 新建缺失包（只建空骨架，已存在则跳过）----------
mk_pkg () {
  local name=$1 layer=$2 desc=$3 deps=$4
  local dir="src/$name"
  if [ -e "$dir" ]; then echo "  [SKIP] $dir 已存在"; return 0; fi
  if [ "$MODE" = "dry" ]; then echo "  [DRY ] 新建包 $dir（$layer）"; return 0; fi
  echo "  [NEW ] 新建包 $dir（$layer）"
  mkdir -p "$dir/include/$name/core" "$dir/src/core" "$dir/src/node" "$dir/test"
  cat > "$dir/package.xml" <<XML
<?xml version="1.0"?>
<?xml-model href="http://download.ros.org/schema/package_format3.xsd" schematypens="http://www.w3.org/2001/XMLSchema"?>
<package format="3">
  <name>$name</name>
  <version>1.3.0</version>
  <description>[$layer] $desc</description>
  <maintainer email="dev@roto.local">roto cart dev</maintainer>
  <license>Proprietary</license>
  <buildtool_depend>ament_cmake</buildtool_depend>
$(printf '  <depend>%s</depend>\n' $deps)
  <test_depend>ament_cmake_gtest</test_depend>
  <export>
    <build_type>ament_cmake</build_type>
  </export>
</package>
XML
  cat > "$dir/CMakeLists.txt" <<CM
cmake_minimum_required(VERSION 3.8)
project($name)
# 分层归属：$layer
# 约束（docs/00-接口裁决.md D6）：src/core 纯 C++（不 include rclcpp）；src/node 只做 ROS 壳
find_package(ament_cmake REQUIRED)
$(printf 'find_package(%s REQUIRED)\n' $deps)
ament_package()
CM
}
echo
echo "-- 新建缺失包（HAL/Service/App）--"
mk_pkg driver_chassis_rs485  "L2-HAL 底盘" "底盘 RS485 控制 + YMODEM OTA" "rclcpp robot_msgs nav_msgs geometry_msgs"
mk_pkg driver_rs485_dev      "L2-HAL 外设" "Modbus RTU 多路外设（COFFEE/ICE/MERCH/FRIDGE/BMS）" "rclcpp robot_msgs"
mk_pkg driver_lidar          "L2-HAL 感知" "激光雷达" "rclcpp robot_msgs sensor_msgs"
mk_pkg driver_audio          "L2-HAL 交互" "语音播报/拾音" "rclcpp robot_msgs"
mk_pkg driver_remote_24g     "L2-HAL 交互" "2.4G 遥控 + 急停" "rclcpp robot_msgs"
mk_pkg driver_npu_perception "L2-HAL 预留" "NPU 感知（RK3588 迁移启用）" "rclcpp robot_msgs"
mk_pkg service_state_mgr     "L3-Service" "整车状态机" "rclcpp robot_msgs"
mk_pkg service_logger        "L3-Service" "日志与云端上报" "rclcpp robot_msgs"
mk_pkg service_config        "L3-Service" "配置热更新" "rclcpp robot_msgs"
mk_pkg app_power_mgr         "L4-App" "电源与充电管理" "rclcpp robot_msgs"
mk_pkg driver_alarm_light    "L2-HAL 交互" "声光报警器（AlarmLightCmd）" "rclcpp robot_msgs"

# ---------- 3.5) 清理已搬空的层目录（git 不跟踪空目录，仅清理本地残留）----------
for d in src/app_layer src/comm_layer src/driver_layer; do
  [ -d "$d" ] || continue
  if [ -z "$(ls -A "$d" 2>/dev/null)" ]; then
    run "rmdir 空目录 $d" rmdir "$d"
  else
    echo "  [KEEP] $d 仍有未迁移包：$(ls -A "$d" | tr '\n' ' ')"
  fi
done

# ---------- 4) 人工待办（脚本不碰）----------
cat <<'TODO'

== 需要人工决定/迁移的（脚本刻意不做）==
  1. driver_can_chassis  vs  §7 的 driver_chassis_rs485：**底盘到底是 CAN 还是 RS485？**
     - 若实际是 CAN：架构 §4.3 的 RS485 帧/YMODEM 口径不适用，属架构偏离，需先裁决
     - 若已切 RS485：driver_can_chassis 代码作参考，新建的 driver_chassis_rs485 承接
  2. driver_bms        → 并入 driver_rs485_dev（BMS-01 是 485 从站 2，不是独立总线）
  3. driver_gpio       → driver_alarm_light（若 gpio 只管声光）；若还控电磁锁/继电器需再分
  4. comm_remote_handle → 并入 service_cloud_comm（远控 WAN→LAN 转发属云端服务职责）
  5. comm_webrtc       → 并入 app_remote_video
  6. app_status_report_node → 拆给 service_state_mgr（状态机）+ service_cloud_comm（上报）
  7. robot_msgs        → 对齐 44 msg / 9 srv（需逐字段 diff 现有 msg）
  8. tools/sim 移出后，检查其 CMake/launch 里对路径的硬编码

== 迁移后必须查的引用 ==
  grep -rnE "app_layer|comm_layer|driver_layer|comm_mqtt_cloud|comm_mqtt_private|ota_manager_node|sim_tools" \
       --include='*.py' --include='*.xml' --include='*.yaml' --include='*.json' --include='*.sh' .
TODO

echo
echo "== 结束（模式：$MODE）=="
[ "$MODE" = "dry" ] && echo "  确认清单无误后执行：./scripts/migrate_layout_v12.sh --apply"
