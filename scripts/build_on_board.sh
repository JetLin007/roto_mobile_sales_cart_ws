#!/usr/bin/env bash
# ==============================================================================
# 文件名：scripts/build_on_board.sh
# 用途：RK3576S 板端原生编译 —— 发布验证通道（git 更新代码 → 板端编译 → 运行）
#      源码仓与运行时根是两个不同概念，本脚本已分开（见 docs/00 D11）：
#        源码仓  : $SRC_DIR      ← git clone 下来的工程（默认脚本上一级目录）
#        运行时根: $RUNTIME_DIR  ← /opt/roto_ws，只放 config/log/data，二进制在镜像里
# 用法：
#   ./scripts/build_on_board.sh --pull            # 先 git pull，再编译（发布验证标准动作）
#   ./scripts/build_on_board.sh -j 4              # 指定并行度（默认 4，防 OOM/降频）
#   ./scripts/build_on_board.sh --require-clean   # 工作区有未提交改动则拒绝编译
#   ./scripts/build_on_board.sh --packages-select robot_msgs
#   ./scripts/build_on_board.sh --clean           # 删 build/ install/ log/ 后全量
#   ./scripts/build_on_board.sh --host            # 宿主直编（仅当宿主已装 ROS2）
#   ./scripts/build_on_board.sh --deploy-config   # 编译后把 config/ 同步到 /opt/roto_ws/config
# ==============================================================================
set -uo pipefail

SRC_DIR_DEFAULT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC_DIR="${SRC_DIR:-$SRC_DIR_DEFAULT}"
RUNTIME_DIR="${RUNTIME_DIR:-/opt/roto_ws}"
JOBS=4
IMAGE="roto-build:arm64"
PKGS=""
CLEAN=0
HOST_MODE=0
FORCE=0
PULL=0
REQUIRE_CLEAN=0
DEPLOY_CONFIG=0
ROS_DISTRO_NAME="${ROS_DISTRO:-humble}"

while [ $# -gt 0 ]; do
  case "$1" in
    -j|--jobs) JOBS="${2:-4}"; shift 2 ;;
    --image) IMAGE="${2:-}"; shift 2 ;;
    --src) SRC_DIR="${2:-}"; shift 2 ;;
    --runtime) RUNTIME_DIR="${2:-}"; shift 2 ;;
    --packages-select) PKGS="${2:-}"; shift 2 ;;
    --clean) CLEAN=1; shift ;;
    --host) HOST_MODE=1; shift ;;
    --force) FORCE=1; shift ;;
    --pull) PULL=1; shift ;;
    --require-clean) REQUIRE_CLEAN=1; shift ;;
    --deploy-config) DEPLOY_CONFIG=1; shift ;;
    -h|--help) sed -n '3,15p' "$0"; exit 0 ;;
    *) echo "未知参数：$1" >&2; exit 2 ;;
  esac
done

ARCH=$(uname -m)
if [ "$ARCH" != "aarch64" ] && [ "$FORCE" -eq 0 ]; then
  echo "[ERR] 当前架构 $ARCH 不是 aarch64；板端编译必须在 RK3576S 上执行（确需模拟加 --force）" >&2
  exit 1
fi
if [ ! -d "$SRC_DIR/src" ]; then
  echo "[ERR] $SRC_DIR 不像工作空间（缺 src/）" >&2; exit 1
fi

# ---- git 版本核对：交叉构建与板端编译必须落在同一 commit ----
GIT_REV="nogit"; GIT_DESC="nogit"; DIRTY=0
if [ -d "$SRC_DIR/.git" ]; then
  if [ "$PULL" -eq 1 ]; then
    echo "[GIT] git pull --ff-only"
    git -C "$SRC_DIR" pull --ff-only || { echo "[ERR] git pull 失败（本地改动或冲突）" >&2; exit 1; }
  fi
  GIT_REV=$(git -C "$SRC_DIR" rev-parse --short HEAD)
  GIT_DESC=$(git -C "$SRC_DIR" describe --tags --always --dirty 2>/dev/null || echo "$GIT_REV")
  DIRTY=$(git -C "$SRC_DIR" status --porcelain | wc -l | tr -d ' ')
fi
if [ "$REQUIRE_CLEAN" -eq 1 ] && [ "$DIRTY" -gt 0 ]; then
  echo "[ERR] 工作区有 $DIRTY 个未提交改动，发布验证要求干净工作区" >&2
  git -C "$SRC_DIR" status --porcelain | sed 's/^/      /' >&2
  exit 1
fi

echo "== 板端编译（发布验证通道）=="
echo "  源码仓   : $SRC_DIR"
echo "  运行时根 : $RUNTIME_DIR  $(test -d "$RUNTIME_DIR" && echo '(存在)' || echo '(尚未创建)')"
echo "  架构     : $ARCH"
echo "  提交     : $GIT_REV（$GIT_DESC，未提交 $DIRTY 项）"
echo "  并行度   : -j$JOBS"
echo "  模式     : $([ "$HOST_MODE" -eq 1 ] && echo 宿主直编 || echo "容器内编译（$IMAGE）")"
echo "  磁盘余量 : $(df -h "$SRC_DIR" | awk 'NR==2{print $4}')"
TEMPS=$(for t in /sys/class/thermal/thermal_zone*/temp; do [ -r "$t" ] && awk '{printf "%d ", $1/1000}' "$t"; done 2>/dev/null)
echo "  当前温度 : ${TEMPS:-未知}"
echo

if [ "$CLEAN" -eq 1 ]; then
  echo "  [CLEAN] 删除 build/ install/ log/"
  rm -rf "${SRC_DIR:?}/build" "${SRC_DIR:?}/install" "${SRC_DIR:?}/log"
fi

COLCON_ARGS=(--symlink-install --parallel-workers "$JOBS"
             --cmake-args -DCMAKE_BUILD_TYPE=Release
                         -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
                         -DCMAKE_C_COMPILER_LAUNCHER=ccache)
[ -n "$PKGS" ] && COLCON_ARGS+=(--packages-select "$PKGS")

START=$(date +%s)

if [ "$HOST_MODE" -eq 1 ]; then
  if [ ! -f "/opt/ros/${ROS_DISTRO_NAME}/setup.bash" ]; then
    echo "[ERR] 宿主无 /opt/ros/${ROS_DISTRO_NAME}，请去掉 --host 走容器编译" >&2
    exit 1
  fi
  # shellcheck disable=SC1090
  source "/opt/ros/${ROS_DISTRO_NAME}/setup.bash"
  cd "$SRC_DIR"
  colcon build "${COLCON_ARGS[@]}"
  RET=$?
else
  if ! command -v docker >/dev/null 2>&1; then
    echo "[ERR] 宿主无 docker，无法容器内编译" >&2; exit 1
  fi
  if ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
    echo "[ERR] 编译镜像不存在：$IMAGE" >&2
    echo "      构建方式：docker build -f docker/Dockerfile.build -t $IMAGE docker/" >&2
    exit 1
  fi
  docker volume create roto-ccache >/dev/null 2>&1 || true
  # --user 与宿主 UID 对齐，避免 install/ 里出现 root 属主文件
  docker run --rm \
    --net=host \
    --ipc=host \
    --user "$(id -u):$(id -g)" \
    -v "${SRC_DIR}:/ws" \
    -v roto-ccache:/ccache \
    -e CCACHE_DIR=/ccache \
    -e CCACHE_MAXSIZE=2G \
    -w /ws \
    "$IMAGE" \
    bash -lc "source /opt/ros/${ROS_DISTRO_NAME}/setup.bash && colcon build ${COLCON_ARGS[*]}"
  RET=$?
fi

END=$(date +%s)
echo
echo "== 结果 =="
echo "  耗时 : $((END-START))s  退出码 : $RET"
echo "  温度 : $(for t in /sys/class/thermal/thermal_zone*/temp; do [ -r "$t" ] && awk '{printf "%d ", $1/1000}' "$t"; done 2>/dev/null)"

if [ "$RET" -eq 0 ]; then
  mkdir -p "$SRC_DIR/install"
  {
    echo "build_time_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo "git_rev=$GIT_REV"
    echo "git_desc=$GIT_DESC"
    echo "uncommitted=$DIRTY"
    echo "src_dir=$SRC_DIR"
    echo "jobs=$JOBS"
    echo "mode=$([ "$HOST_MODE" -eq 1 ] && echo host || echo container:$IMAGE)"
    echo "arch=$ARCH"
  } > "$SRC_DIR/install/BUILD_INFO.txt"
  echo "  版本戳 : $SRC_DIR/install/BUILD_INFO.txt（$GIT_DESC）"

  # 运行时根的 config 只由发布动作写入，不从构建目录直接挂载
  if [ "$DEPLOY_CONFIG" -eq 1 ]; then
    mkdir -p "$RUNTIME_DIR/config"
    cp -f "$SRC_DIR"/config/*.json "$RUNTIME_DIR/config/" 2>/dev/null || true
    echo "  配置部署 : $RUNTIME_DIR/config ← $SRC_DIR/config（$(ls -1 "$RUNTIME_DIR/config" 2>/dev/null | wc -l) 个文件）"
  fi
  echo "  下一步 : source install/setup.bash && ros2 interface list | grep -c robot_msgs"
fi
exit "$RET"
