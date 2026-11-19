#!/usr/bin/env bash
# ==============================================================================
# 文件名：scripts/deploy_to_board.sh
# 用途：把本地构建的 arm64 镜像部署到 RK3576S 开发板
#      流程：docker save → scp → ssh docker load → （可选）docker run
#      遵循架构 D11：容器只挂载 config/log/data 三个可变目录，二进制随镜像。
# 用法：
#   ./scripts/deploy_to_board.sh 192.168.1.50 roto-cart:1.3.0-arm64
#   ./scripts/deploy_to_board.sh 192.168.1.50 roto-cart:1.3.0-arm64 --run
#   ./scripts/deploy_to_board.sh 192.168.1.50 roto-cart:1.3.0-arm64 --run --name roto-cart
#   ./scripts/deploy_to_board.sh 192.168.1.50 -u root --remote-dir /data/roto
#   ./scripts/deploy_to_board.sh 192.168.1.50 roto-cart:1.3.0-arm64 --dry-run
# ==============================================================================
set -uo pipefail

WS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# ---- 默认参数 ----
TARGET=""
IMAGE=""
USER_NAME="roto"
REMOTE_DIR="/opt/roto_ws"
CONTAINER_NAME="roto-cart"
RUN_CONTAINER=0
DRYRUN=0
ROS_DOMAIN_ID="${ROS_DOMAIN_ID:-0}"
RMW_IMPLEMENTATION="${RMW_IMPLEMENTATION:-rmw_cyclonedds_cpp}"

usage() { sed -n '3,14p' "$0"; }

# ---- 参数解析 ----
if [ $# -lt 1 ]; then usage; exit 2; fi
TARGET="$1"; shift
if [ $# -ge 1 ] && [[ "$1" != --* ]]; then
  IMAGE="$1"; shift
fi

while [ $# -gt 0 ]; do
  case "$1" in
    -u|--user)          USER_NAME="${2:-roto}"; shift 2 ;;
    --remote-dir)       REMOTE_DIR="${2:-/opt/roto_ws}"; shift 2 ;;
    --name)             CONTAINER_NAME="${2:-roto-cart}"; shift 2 ;;
    --run)              RUN_CONTAINER=1; shift ;;
    --domain-id)        ROS_DOMAIN_ID="${2:-0}"; shift 2 ;;
    --rmw)              RMW_IMPLEMENTATION="${2:-rmw_cyclonedds_cpp}"; shift 2 ;;
    --dry-run)          DRYRUN=1; shift ;;
    -h|--help)          usage; exit 0 ;;
    *) echo "[ERR] 未知参数：$1" >&2; usage; exit 2 ;;
  esac
done

if [ -z "$TARGET" ]; then
  echo "[ERR] 缺少板子地址" >&2; usage; exit 2
fi
if [ -z "$IMAGE" ]; then
  # 尝试用最新构建的镜像
  IMAGE=$(docker images --format '{{.Repository}}:{{.Tag}}' 2>/dev/null | grep -E 'roto-cart|roto_sales_cart' | head -1)
  if [ -z "$IMAGE" ]; then
    echo "[ERR] 缺少镜像名，且本地未找到 roto-cart / roto_sales_cart 镜像" >&2
    echo "      用法: $0 <板子IP> <镜像名>" >&2; exit 2
  fi
  echo "[INFO] 未指定镜像，自动选用：$IMAGE"
fi

# ---- 前置检查 ----
if ! command -v docker >/dev/null 2>&1; then
  echo "[ERR] 未安装 docker" >&2; exit 1
fi
if ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
  echo "[ERR] 本地镜像不存在：$IMAGE" >&2
  echo "      请先执行：./scripts/build_image.sh" >&2; exit 1
fi
# 确认是 arm64 镜像
IMG_ARCH=$(docker image inspect "$IMAGE" --format '{{.Architecture}}' 2>/dev/null)
if [ "$IMG_ARCH" != "arm64" ] && [ "$IMG_ARCH" != "aarch64" ]; then
  echo "[WARN] 镜像架构是 $IMG_ARCH，不是 arm64，RK3576S 上无法运行" >&2
fi

echo "== 部署到板子 =="
echo "  目标   : ${USER_NAME}@${TARGET}"
echo "  镜像   : $IMAGE（架构 $IMG_ARCH）"
echo "  远程目录: $REMOTE_DIR"
[ "$RUN_CONTAINER" -eq 1 ] && echo "  启动   : 是（容器名 $CONTAINER_NAME）"
[ "$DRYRUN" -eq 1 ] && echo "  模式   : DRY-RUN"
echo

# SSH 连通性检查（dry-run 跳过）
if [ "$DRYRUN" -eq 0 ]; then
  if ! ssh -o BatchMode=yes -o ConnectTimeout=5 "${USER_NAME}@${TARGET}" "true" 2>/dev/null; then
    echo "[ERR] SSH 免密未就绪：ssh-copy-id ${USER_NAME}@${TARGET}" >&2
    exit 1
  fi
fi

# 远程目录确认（板端需要 sudo 权限创建 /opt/roto_ws）
if [ "$DRYRUN" -eq 1 ]; then
  REMOTE_HOME="\$HOME"
else
  REMOTE_HOME=$(ssh "${USER_NAME}@${TARGET}" 'echo $HOME' 2>/dev/null)
  echo "  板端家目录: $REMOTE_HOME"
fi

# 临时 tar 包名
TARBALL="$(echo "$IMAGE" | tr '/:' '__').tar"
LOCAL_TAR="/tmp/$TARBALL"
REMOTE_TAR="$REMOTE_HOME/$TARBALL"

run_remote() {
  local cmd="$1"
  if [ "$DRYRUN" -eq 1 ]; then
    echo "  [DRY-RUN] ssh ${USER_NAME}@${TARGET} '$cmd'"
  else
    ssh "${USER_NAME}@${TARGET}" "$cmd"
  fi
}

# ---- 步骤 1：docker save ----
echo "== 1/4 打包镜像 =="
if [ "$DRYRUN" -eq 1 ]; then
  echo "  [DRY-RUN] docker save $IMAGE | gzip > $LOCAL_TAR"
else
  echo "  docker save $IMAGE | gzip > $LOCAL_TAR"
  docker save "$IMAGE" | gzip > "$LOCAL_TAR"
  SIZE=$(du -h "$LOCAL_TAR" | awk '{print $1}')
  echo "  完成，大小 $SIZE"
fi

# ---- 步骤 2：scp 到板子 ----
echo
echo "== 2/4 传输到板子 =="
if [ "$DRYRUN" -eq 1 ]; then
  echo "  [DRY-RUN] scp $LOCAL_TAR ${USER_NAME}@${TARGET}:$REMOTE_TAR"
else
  scp "$LOCAL_TAR" "${USER_NAME}@${TARGET}:${REMOTE_TAR}"
  echo "  传输完成"
fi

# ---- 步骤 3：ssh docker load ----
echo
echo "== 3/4 板端加载镜像 =="
run_remote "gunzip -c $REMOTE_TAR | docker load && rm -f $REMOTE_TAR && docker images | grep -E '$(echo "$IMAGE" | cut -d: -f1)'"

# ---- 步骤 4：（可选）启动容器 ----
if [ "$RUN_CONTAINER" -eq 1 ]; then
  echo
  echo "== 4/4 启动容器 =="
  # 板端创建挂载目录（需要 sudo）
  run_remote "sudo mkdir -p $REMOTE_DIR/config $REMOTE_DIR/log $REMOTE_DIR/data"

  # 停掉旧容器
  run_remote "docker rm -f $CONTAINER_NAME 2>/dev/null || true"

  # 启动新容器
  RUN_CMD="docker run -d --name $CONTAINER_NAME \
    --restart unless-stopped \
    --net=host --ipc=host \
    -v $REMOTE_DIR/config:/opt/roto_ws/config \
    -v $REMOTE_DIR/log:/opt/roto_ws/log \
    -v $REMOTE_DIR/data:/opt/roto_ws/data \
    -e ROS_DOMAIN_ID=$ROS_DOMAIN_ID \
    -e RMW_IMPLEMENTATION=$RMW_IMPLEMENTATION \
    $IMAGE"

  if [ "$DRYRUN" -eq 1 ]; then
    echo "  [DRY-RUN] $RUN_CMD"
  else
    ssh "${USER_NAME}@${TARGET}" "$RUN_CMD"
    echo
    echo "  容器状态："
    ssh "${USER_NAME}@${TARGET}" "docker ps --filter name=$CONTAINER_NAME --format 'table {{.Names}}\t{{.Status}}\t{{.Image}}'"
    echo
    echo "  查看日志：ssh ${USER_NAME}@${TARGET} 'docker logs -f $CONTAINER_NAME'"
  fi
else
  echo
  echo "  跳过启动容器（未指定 --run）"
  echo "  板端手动启动："
  echo "    ssh ${USER_NAME}@${TARGET}"
  echo "    docker run -d --name $CONTAINER_NAME --restart unless-stopped --net=host --ipc=host \\"
  echo "      -v $REMOTE_DIR/config:/opt/roto_ws/config \\"
  echo "      -v $REMOTE_DIR/log:/opt/roto_ws/log \\"
  echo "      -v $REMOTE_DIR/data:/opt/roto_ws/data \\"
  echo "      -e ROS_DOMAIN_ID=$ROS_DOMAIN_ID \\"
  echo "      -e RMW_IMPLEMENTATION=$RMW_IMPLEMENTATION \\"
  echo "      $IMAGE"
fi

# 清理本地临时文件
[ "$DRYRUN" -eq 0 ] && rm -f "$LOCAL_TAR"
echo
echo "  部署完成。"
