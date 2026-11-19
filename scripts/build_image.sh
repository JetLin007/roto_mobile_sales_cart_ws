#!/usr/bin/env bash
# ==============================================================================
# 文件名：scripts/build_image.sh
# 用途：交叉构建 RK3576S (arm64) 发布镜像，支持离线 tar 导出与多架构 manifest
#      遵循架构 D8/D10 裁决：
#        - 单架构调试镜像必须带 -arm64 / -amd64 后缀
#        - 发布用多架构 manifest（无后缀）
#        - 镜像 OCI label 写入 org.opencontainers.image.revision=git_rev
# 用法：
#   ./scripts/build_image.sh                              # 构建 arm64，--load 到本地
#   ./scripts/build_image.sh -t myreg/roto-cart:1.3.0-arm64
#   ./scripts/build_image.sh --tar out/roto-cart-arm64.tar   # 离线包（板端 docker load）
#   ./scripts/build_image.sh --multiarch                    # amd64+arm64 manifest，--push
#   ./scripts/build_image.sh --amd64                        # 本机 amd64 验证
#   ./scripts/build_image.sh --push                         # push 到 registry
#   ./scripts/build_image.sh --dry-run                      # 只打印命令不执行
# ==============================================================================
set -uo pipefail

WS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$WS_DIR"

# ---- 默认参数 ----
PLATFORM="linux/arm64"
TAG_SUFFIX="-arm64"
OUTPUT="--load"            # 默认 load 到本地 docker
TAR_PATH=""
MULTIARCH=0
PUSH=0
DRYRUN=0
REGISTRY=""
IMAGE_NAME="roto-cart"
GIT_TAG=""

# ---- git 版本信息（用于 tag 与 OCI label）----
GIT_REV="nogit"
GIT_DESC="nogit"
DIRTY=0
if [ -d ".git" ]; then
  GIT_REV=$(git rev-parse --short HEAD 2>/dev/null || echo "nogit")
  GIT_DESC=$(git describe --tags --always --dirty 2>/dev/null || echo "$GIT_REV")
  DIRTY=$(git status --porcelain | wc -l | tr -d ' ')
fi

usage() { sed -n '3,18p' "$0"; }

# ---- 参数解析 ----
while [ $# -gt 0 ]; do
  case "$1" in
    -t|--tag)          GIT_TAG="${2:-}"; shift 2 ;;
    --registry)        REGISTRY="${2:-}/"; shift 2 ;;
    --name)            IMAGE_NAME="${2:-roto-cart}"; shift 2 ;;
    --platform)        PLATFORM="${2:-linux/arm64}"; shift 2 ;;
    --arm64)           PLATFORM="linux/arm64"; TAG_SUFFIX="-arm64"; shift ;;
    --amd64)           PLATFORM="linux/amd64"; TAG_SUFFIX="-amd64"; shift ;;
    --multiarch)       MULTIARCH=1; PLATFORM="linux/amd64,linux/arm64"; TAG_SUFFIX=""; PUSH=1; shift ;;
    --tar)             TAR_PATH="${2:-}"; OUTPUT="--output type=docker,dest=${TAR_PATH}"; shift 2 ;;
    --push)            PUSH=1; OUTPUT="--push"; shift ;;
    --load)            OUTPUT="--load"; shift ;;
    --dry-run)         DRYRUN=1; shift ;;
    -h|--help)         usage; exit 0 ;;
    *) echo "[ERR] 未知参数：$1" >&2; usage; exit 2 ;;
  esac
done

# ---- 构造 tag ----
if [ -z "$GIT_TAG" ]; then
  # 取 git describe 里的版本号，去掉 v 前缀和 dirty 标记
  VER=$(echo "$GIT_DESC" | sed 's/^v//;s/-[0-9]*-g[0-9a-f]*//;s/-dirty//')
  [ -z "$VER" ] && VER="$GIT_REV"
  GIT_TAG="${VER}${TAG_SUFFIX}"
fi
FULL_TAG="${REGISTRY}${IMAGE_NAME}:${GIT_TAG}"

# ---- 前置检查 ----
if ! command -v docker >/dev/null 2>&1; then
  echo "[ERR] 未安装 docker" >&2; exit 1
fi
if ! docker buildx version >/dev/null 2>&1; then
  echo "[ERR] docker buildx 不可用" >&2; exit 1
fi
if [ ! -f "$WS_DIR/Dockerfile" ]; then
  echo "[ERR] 找不到 Dockerfile：$WS_DIR/Dockerfile" >&2; exit 1
fi
if [ "$DIRTY" -gt 0 ] && [ "$MULTIARCH" -eq 1 ]; then
  echo "[ERR] 发布多架构镜像要求工作区干净（当前 $DIRTY 项未提交）" >&2
  echo "      请先 git commit 或 stash，或用 --arm64/--amd64 单架构调试" >&2
  exit 1
fi

# ---- tar 输出目录 ----
if [ -n "$TAR_PATH" ]; then
  TAR_DIR="$(dirname "$TAR_PATH")"
  mkdir -p "$TAR_DIR"
fi

echo "== 构建镜像 =="
echo "  工作区   : $WS_DIR"
echo "  平台     : $PLATFORM"
echo "  镜像     : $FULL_TAG"
echo "  输出     : $([ "$PUSH" -eq 1 ] && echo push || ([ -n "$TAR_PATH" ] && echo "tar -> $TAR_PATH" || echo load))"
echo "  提交     : $GIT_REV（$GIT_DESC，未提交 $DIRTY 项）"
[ "$MULTIARCH" -eq 1 ] && echo "  模式     : 多架构 manifest"
[ "$DRYRUN" -eq 1 ] && echo "  模式     : DRY-RUN（不实际执行）"
echo

BUILDX_ARGS=(buildx build
  --platform "$PLATFORM"
  -t "$FULL_TAG"
  --label "org.opencontainers.image.revision=$GIT_REV"
  --label "org.opencontainers.image.source=roto_mobile_sales_cart_ws"
  --label "org.opencontainers.image.created=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
)
[ -n "$OUTPUT" ] && BUILDX_ARGS+=($OUTPUT)
BUILDX_ARGS+=("$WS_DIR")

if [ "$DRYRUN" -eq 1 ]; then
  echo "  [DRY-RUN] docker ${BUILDX_ARGS[*]}"
  exit 0
fi

START=$(date +%s)
docker "${BUILDX_ARGS[@]}"
RET=$?
END=$(date +%s)

echo
echo "== 结果 =="
echo "  耗时     : $((END-START))s  退出码 : $RET"
echo "  镜像     : $FULL_TAG"
if [ $RET -eq 0 ]; then
  if [ -n "$TAR_PATH" ]; then
    SIZE=$(du -h "$TAR_PATH" 2>/dev/null | awk '{print $1}')
    echo "  离线包   : $TAR_PATH（$SIZE）"
    echo "  板端加载 : docker load -i $(basename "$TAR_PATH")"
  elif [ "$PUSH" -eq 1 ]; then
    echo "  已 push，板端执行: docker pull $FULL_TAG"
  else
    echo "  已 load，本地可用: docker images $FULL_TAG"
  fi
fi
exit $RET
