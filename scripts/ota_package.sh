#!/usr/bin/env bash
# =============================================================================
# ota_package.sh — 端到端 OTA 打包脚本
# -----------------------------------------------------------------------------
# 把运行中的 roto 容器打包成可云端下发的 OTA tarball：
#
#   1) docker commit 运行中的容器 → 带 OCI labels 的新镜像
#   2) docker save | gzip → tarball
#   3) sha256 + Ed25519 签名
#   4) 生成 ota/app/version JSON（云端 MQTT 下发完整 payload）
#   5) 输出 manifest.json 汇总包信息
#
# 输出目录默认: /opt/roto_ws/data/ota/cache/
#   （容器内 OtaCore.download_dir 默认也是这里，双方天然对齐）
#
# 用法:
#   # 用默认容器名 roto-builder，版本自动取 git describe
#   bash scripts/ota_package.sh
#
#   # 指定版本 + 输出目录
#   bash scripts/ota_package.sh --version 1.2.0 --container roto-builder
#
#   # 指定 Ed25519 私钥（不指定则自动生成一对）
#   bash scripts/ota_package.sh --priv-key /opt/roto_ws/data/ota/ed25519_priv.pem
# =============================================================================
set -euo pipefail

# ---- 默认值 ----
CONTAINER="${OTA_CONTAINER:-roto-builder}"
OUT_DIR="${OTA_OUT_DIR:-/opt/roto_ws/data/ota/cache}"
VERSION=""
PRIV_KEY=""
SIGN_ONLY=0  # 已存在 tarball，只算 sha256 + 签名

usage() {
  sed -n '2,22p' "$0"
  exit "${1:-0}"
}

# ---- 解析参数 ----
while [[ $# -gt 0 ]]; do
  case "$1" in
    -h|--help) usage 0 ;;
    -c|--container)    CONTAINER="$2"; shift 2 ;;
    -v|--version)      VERSION="$2"; shift 2 ;;
    -o|--out-dir)      OUT_DIR="$2"; shift 2 ;;
    -k|--priv-key)     PRIV_KEY="$2"; shift 2 ;;
    -s|--sign-only)    SIGN_ONLY=1; shift ;;
    *) echo "未知参数: $1"; usage 1 ;;
  esac
done

# ---- 0) 环境预检 ----
command -v docker   >/dev/null 2>&1 || { echo "❌ docker 未安装"; exit 1; }
command -v openssl  >/dev/null 2>&1 || { echo "❌ openssl 未安装"; exit 1; }

if [[ "$SIGN_ONLY" -eq 0 ]]; then
  docker ps --filter "name=${CONTAINER}" --format '{{.Names}}' | grep -q "${CONTAINER}" \
    || { echo "❌ 容器 ${CONTAINER} 没在跑 (docker ps)"; exit 1; }
fi

mkdir -p "${OUT_DIR}"

# ---- 1) 版本号 (git describe → fallback to date) ----
if [[ -z "$VERSION" ]]; then
  VERSION=$(git -C "$(dirname "$0")/.." describe --tags --always --dirty 2>/dev/null || true)
  VERSION="${VERSION:-$(date +%Y%m%d%H%M%S)}"
fi
echo "📦 版本: ${VERSION}"

TARBALL="${OUT_DIR}/roto-cart-v${VERSION}.tar.gz"
MANIFEST="${OUT_DIR}/manifest-v${VERSION}.json"
OCI_IMAGE="roto-cart:${VERSION}"

# ---- 2) OCI revision (git commit hash → fallback to "local-build") ----
REVISION=$(git -C "$(dirname "$0")/.." rev-parse HEAD 2>/dev/null || echo "local-build")
CREATED_AT=$(date -u +"%Y-%m-%dT%H:%M:%SZ")

# ---- 3) docker commit 运行中容器 → 带 OCI labels ----
if [[ "$SIGN_ONLY" -eq 0 ]]; then
  echo "🍳 docker commit ${CONTAINER} → ${OCI_IMAGE} (带 OCI labels)"
  docker commit \
    --change "LABEL org.opencontainers.image.revision=${REVISION}" \
    --change "LABEL org.opencontainers.image.version=${VERSION}" \
    --change "LABEL org.opencontainers.image.created=${CREATED_AT}" \
    --change "LABEL org.opencontainers.image.source=https://git.example.com/roto/roto-cart" \
    --change "LABEL org.opencontainers.image.title=roto-cart" \
    --change 'LABEL org.opencontainers.image.description="roto-cart mobile base runtime"' \
    "${CONTAINER}" "${OCI_IMAGE}"
  ok_image="✅ commit 完成: ${OCI_IMAGE}"
else
  echo "⏭  SIGN_ONLY 跳过 docker commit"
fi

# ---- 4) docker save → tarball ----
if [[ "$SIGN_ONLY" -eq 0 ]]; then
  echo "🗜  docker save ${OCI_IMAGE} | gzip → ${TARBALL}"
  docker save "${OCI_IMAGE}" | gzip > "${TARBALL}"
  echo "   $(ls -lh "${TARBALL}" | awk '{print $5}')"
else
  [[ -f "${TARBALL}" ]] || { echo "❌ SIGN_ONLY 但 ${TARBALL} 不存在"; exit 1; }
  echo "⏭  SIGN_ONLY 跳过 save (复用 ${TARBALL})"
fi

# ---- 5) sha256 + Ed25519 签名 ----
SHA256=$(sha256sum "${TARBALL}" | awk '{print $1}')
echo "🔐 sha256  = ${SHA256}"

KEY_DIR="$(dirname "${TARBALL}")"
if [[ -z "${PRIV_KEY}" ]]; then
  PRIV_KEY="${KEY_DIR}/ed25519_priv.pem"
fi
PUB_KEY="${KEY_DIR}/ed25519_pub.pem"

if [[ ! -f "${PRIV_KEY}" ]]; then
  echo "🔑 自动生成 Ed25519 密钥对 → ${KEY_DIR}/ed25519_*.pem"
  openssl genpkey -algorithm Ed25519 -out "${PRIV_KEY}" 2>/dev/null
  openssl pkey -in "${PRIV_KEY}" -pubout -out "${PUB_KEY}" 2>/dev/null
fi
openssl pkey -in "${PRIV_KEY}" -pubout -out "${PUB_KEY}" 2>/dev/null

SIG_B64=$(openssl pkeyutl -sign -inkey "${PRIV_KEY}" -in "${TARBALL}" -rawin -out - \
  | base64 -w0)
echo "✍️  signature (base64): ${SIG_B64:0:40}..."

# ---- 6) 生成 ota/app/version JSON (MQTT 下发 payload) ----
PUB_PEM=$(cat "${PUB_KEY}")
MQTT_JSON=$(cat <<JSONEOF
{
  "msg_id": "uuid-$(date +%s)-${RANDOM}",
  "timestamp": $(date +%s)000,
  "ota_task_id": "task-$(date +%Y%m%d)-${VERSION}",
  "target_module": "APP",
  "target_version": "${VERSION}",
  "image_url": "http://<cloud-registry>/roto-cart-v${VERSION}.tar.gz",
  "sha256": "${SHA256}",
  "signature": "${SIG_B64}",
  "delivery": "tar",
  "min_battery_soc": 20
}
JSONEOF
)

# ---- 7) 输出 manifest.json ----
cat > "${MANIFEST}" <<JSONEOF
{
  "package": "roto-cart",
  "version": "${VERSION}",
  "revision": "${REVISION}",
  "created_at": "${CREATED_AT}",
  "tarball": {
    "path": "${TARBALL}",
    "sha256": "${SHA256}",
    "signature_base64": "${SIG_B64}",
    "size_bytes": $(stat -c%s "${TARBALL}"),
    "ed25519_pubkey_pem": $(echo "${PUB_PEM}" | python3 -c 'import sys,json; print(json.dumps(sys.stdin.read()))')
  },
  "oci_labels": {
    "org.opencontainers.image.revision": "${REVISION}",
    "org.opencontainers.image.version": "${VERSION}",
    "org.opencontainers.image.created": "${CREATED_AT}"
  },
  "mqtt_payload": ${MQTT_JSON},
  "usage": {
    "agent_command": "OTA_CONTAINER=roto-builder python3 scripts/cart_ota_agent.py -d /opt/roto_ws/data/ota",
    "ota_app_param_example": "--ros-args --params-file params.yaml (ed25519_pubkey_pem 字段用上面的公钥)",
    "mqtt_topics": ["ota/app/version", "ota/chassis/version", "ota/status"]
  }
}
JSONEOF

# ---- 8) 打印汇总 ----
cat <<EOF

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅  OTA 打包完成
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  容器:      ${CONTAINER}
  版本:      ${VERSION}
  revision:  ${REVISION}
  OCI 镜像:  ${OCI_IMAGE}
  tarball:   ${TARBALL}
  sha256:    ${SHA256}
  签名:      ${SIG_B64:0:40}...
  公钥:      ${PUB_KEY}
  manifest:  ${MANIFEST}

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📡  云端 MQTT 下发 (ota/app/version)：
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

${MQTT_JSON}

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📋  OTA 指令摘要 (manual publish 示例)：
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

mosquitto_pub -h <broker> -p 1883 -t ota/app/version \\
  -d '${MQTT_JSON}'

EOF
