#!/usr/bin/env bash
# ==============================================================================
# 文件名：scripts/sync_to_board.sh
# 用途：WSL2 编辑 → RK3576S 板端编译 的代码同步（只传源码，不传构建产物）
# 用法：./scripts/sync_to_board.sh <板子IP或主机名> [用户名] [--delete] [--dry-run]
# 默认：用户 roto、目标目录 /opt/roto_ws、排除 build/install/log/.git
# --delete：删除目标端多余文件（首次慎用，会删掉板子上手工改的东西）
# ==============================================================================
set -uo pipefail

if [ $# -lt 1 ]; then
  sed -n '3,8p' "$0"; exit 2
fi

TARGET="$1"; shift
USER_NAME="roto"
DELETE_FLAG=""
DRYRUN=""
while [ $# -gt 0 ]; do
  case "$1" in
    --delete) DELETE_FLAG="--delete" ;;
    --dry-run) DRYRUN="--dry-run" ;;
    *) if [ -z "${USER_NAME_SET:-}" ] && [ "$1" != "--delete" ] && [ "$1" != "--dry-run" ]; then USER_NAME="$1"; USER_NAME_SET=1; fi ;;
  esac
  shift
done

WS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
REMOTE_DIR="${REMOTE_DIR:-/opt/roto_ws}"

echo "== 同步 =="
echo "  源   : $WS_DIR"
echo "  目标 : ${USER_NAME}@${TARGET}:${REMOTE_DIR}"
[ -n "$DELETE_FLAG" ] && echo "  模式 : 镜像删除（目标端多余文件会被清掉）"
[ -n "$DRYRUN" ] && echo "  模式 : dry-run（不实际写入）"
echo

# 先确认板子可达，避免 rsync 卡在认证
if ! ssh -o BatchMode=yes -o ConnectTimeout=5 "${USER_NAME}@${TARGET}" "true" 2>/dev/null; then
  echo "[ERR] SSH 免密未就绪：ssh-copy-id ${USER_NAME}@${TARGET}" >&2
  exit 1
fi

rsync -az --info=progress2 $DRYRUN $DELETE_FLAG \
  --exclude 'build/' \
  --exclude 'install/' \
  --exclude 'log/' \
  --exclude '.git/' \
  --exclude '*.pyc' \
  --exclude '__pycache__/' \
  "$WS_DIR"/ "${USER_NAME}@${TARGET}:${REMOTE_DIR}/"

echo
echo "  同步完成，下一步在板子上执行："
echo "    ssh ${USER_NAME}@${TARGET} 'cd ${REMOTE_DIR} && ./scripts/build_on_board.sh -j 4'"
