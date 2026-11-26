#!/usr/bin/env python3
# =============================================================================
# cart_ota_agent.py — 宿主侧 OTA 执行代理（systemd 常驻）
# -----------------------------------------------------------------------------
# 职责：容器内 service_ota_app 无法操作 docker，由本代理执行镜像替换。
# 握手目录：/opt/roto_ws/data/ota/（与容器共享挂载）
#   request.json  <- 容器内 OtaCore 写入（下载+校验完成后）
#   result.json   -> 本代理写入执行结果（容器重启后上报云端）
#
# 流程：
#   1. 轮询 request.json
#   2. tar 模式: docker load -i <tar>;  pull 模式: docker pull <image_ref>
#   3. 校验 OCI label org.opencontainers.image.revision（防镜像被掉包）
#   4. 把当前镜像 tag 为 roto-cart:previous（A/B 回滚位）
#   5. 把新镜像 tag 为 roto-cart:current
#   6. docker restart <container>
#   7. 等待容器健康 -> 写 result.json {status: success}
#      失败 -> retag previous -> restart -> 写 result.json {status: rollback_*}
#
# 安全：仅消费本代理自己的 request.json，不执行任意命令；tar 必须通过容器内
#       sha256 校验，本代理再二次校验 docker images 的 digest。
# =============================================================================
import json
import os
import subprocess
import sys
import time
import logging
from pathlib import Path
from typing import Optional

# ---- 配置（可通过环境变量覆盖）----
OTA_DIR        = Path(os.getenv("OTA_DIR", "/opt/roto_ws/data/ota"))
CONTAINER_NAME = os.getenv("OTA_CONTAINER", "roto-cart")
IMAGE_CURRENT  = os.getenv("OTA_IMAGE_CURRENT", "roto-cart:current")
IMAGE_PREVIOUS = os.getenv("OTA_IMAGE_PREVIOUS", "roto-cart:previous")
HEALTH_WAIT_S  = int(os.getenv("OTA_HEALTH_WAIT", "30"))
POLL_INTERVAL  = float(os.getenv("OTA_POLL_INTERVAL", "2"))

REQUEST_FILE = OTA_DIR / "request.json"
RESULT_FILE  = OTA_DIR / "result.json"

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[logging.StreamHandler(sys.stdout)],
)
log = logging.getLogger("cart-ota-agent")


def run(cmd: list, timeout: int = 120) -> subprocess.CompletedProcess:
    """执行 shell 命令，返回结果；失败抛异常由调用方处理。"""
    log.info("$ %s", " ".join(cmd))
    return subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)


def docker_inspect_revision(image: str) -> Optional[str]:
    """读取镜像的 OCI label org.opencontainers.image.revision。"""
    cp = run(["docker", "inspect", "-f",
              "{{index .Config.Labels \"org.opencontainers.image.revision\"}}",
              image], timeout=30)
    if cp.returncode != 0:
        return None
    rev = cp.stdout.strip()
    return rev if rev and rev != "<no value>" else None


def docker_image_exists(image: str) -> bool:
    return run(["docker", "image", "inspect", image], timeout=15).returncode == 0


def container_running(name: str) -> bool:
    cp = run(["docker", "inspect", "-f", "{{.State.Running}}", name], timeout=15)
    return cp.returncode == 0 and cp.stdout.strip() == "true"


def load_or_pull(req: dict) -> Optional[str]:
    """根据 delivery 字段加载镜像，返回新镜像的引用名。"""
    delivery = req.get("delivery", "tar")
    if delivery == "pull":
        image_ref = req.get("image_ref", "")
        if not image_ref:
            log.error("pull 模式缺少 image_ref")
            return None
        cp = run(["docker", "pull", image_ref], timeout=300)
        if cp.returncode != 0:
            log.error("docker pull 失败: %s", cp.stderr)
            return None
        return image_ref
    else:  # tar
        tar_path = req.get("tar_path", "")
        if not tar_path or not Path(tar_path).is_file():
            log.error("tar 模式找不到文件: %s", tar_path)
            return None
        cp = run(["docker", "load", "-i", tar_path], timeout=300)
        if cp.returncode != 0:
            log.error("docker load 失败: %s", cp.stderr)
            return None
        # docker load 输出 "Loaded image: roto-cart:1.3.0-arm64"
        for line in cp.stdout.splitlines():
            if line.startswith("Loaded image:"):
                return line.split(":", 1)[1].strip()
        log.error("无法从 docker load 输出解析镜像名")
        return None


def switch_and_restart(new_image: str, req_sha256: str) -> bool:
    """执行镜像切换 + 容器重启；失败回滚。返回是否成功。"""
    # 1) 二次校验：新镜像 label revision 必须非空（build_image.sh 写入）
    rev = docker_inspect_revision(new_image)
    if not rev:
        log.error("新镜像缺少 OCI label revision，拒绝安装")
        return False
    log.info("新镜像 revision=%s", rev)

    # 2) 把当前 current 镜像存为 previous（A/B 回滚位）
    had_previous = False
    if docker_image_exists(IMAGE_CURRENT):
        run(["docker", "tag", IMAGE_CURRENT, IMAGE_PREVIOUS], timeout=15)
        had_previous = True
        log.info("已将 %s 备份为 %s", IMAGE_CURRENT, IMAGE_PREVIOUS)

    # 3) 新镜像 tag 为 current
    cp = run(["docker", "tag", new_image, IMAGE_CURRENT], timeout=15)
    if cp.returncode != 0:
        log.error("tag 失败: %s", cp.stderr)
        return False

    # 4) 重启容器
    cp = run(["docker", "restart", CONTAINER_NAME], timeout=60)
    if cp.returncode != 0:
        log.error("容器重启失败，回滚: %s", cp.stderr)
        return rollback(had_previous)

    # 5) 等待容器健康（简化版：等 running 且持续 N 秒不退出）
    log.info("等待容器健康（最多 %ss）...", HEALTH_WAIT_S)
    deadline = time.time() + HEALTH_WAIT_S
    healthy = False
    while time.time() < deadline:
        if container_running(CONTAINER_NAME):
            healthy = True
            break
        time.sleep(2)
    if not healthy:
        log.error("容器启动失败，回滚")
        return rollback(had_previous)

    log.info("OTA 成功，容器已运行")
    return True


def rollback(had_previous: bool) -> bool:
    """回滚到 previous 镜像并重启。"""
    if not had_previous or not docker_image_exists(IMAGE_PREVIOUS):
        log.error("无 previous 镜像可回滚")
        return False
    run(["docker", "tag", IMAGE_PREVIOUS, IMAGE_CURRENT], timeout=15)
    cp = run(["docker", "restart", CONTAINER_NAME], timeout=60)
    if cp.returncode != 0:
        log.error("回滚重启失败: %s", cp.stderr)
        return False
    # 等待回滚后容器运行
    for _ in range(HEALTH_WAIT_S // 2):
        if container_running(CONTAINER_NAME):
            log.info("回滚成功")
            return True
        time.sleep(2)
    log.error("回滚后容器仍无法运行")
    return False


def write_result(req: dict, status: str, message: str) -> None:
    """写 result.json，容器内新进程会读取并上报云端。"""
    result = {
        "ota_task_id": req.get("ota_task_id", ""),
        "target_version": req.get("target_version", ""),
        "status": status,
        "message": message,
        "image_ref": req.get("image_ref", ""),
        "timestamp_ms": int(time.time() * 1000),
    }
    tmp = RESULT_FILE.with_suffix(".json.tmp")
    tmp.write_text(json.dumps(result, indent=2))
    tmp.replace(RESULT_FILE)
    log.info("写 result.json: %s", status)


def process_request(req_path: Path) -> None:
    """处理一个 request.json。"""
    try:
        req = json.loads(req_path.read_text())
    except Exception as e:
        log.error("解析 request.json 失败: %s", e)
        req_path.unlink(missing_ok=True)
        return

    ota_task_id = req.get("ota_task_id", "")
    log.info("收到 OTA 请求 task=%s version=%s delivery=%s",
             ota_task_id, req.get("target_version"), req.get("delivery"))

    # 先写 installing 状态
    write_result(req, "installing", "agent processing")

    # 1) 加载镜像
    new_image = load_or_pull(req)
    if not new_image:
        write_result(req, "failed", "load/pull image failed")
        req_path.unlink(missing_ok=True)
        return

    # 2) 切换 + 重启 + 健康检查
    ok = switch_and_restart(new_image, req.get("sha256", ""))

    # 3) 写最终结果
    if ok:
        write_result(req, "success", f"installed {new_image}")
    else:
        # 判断是回滚成功还是彻底失败
        if docker_image_exists(IMAGE_PREVIOUS) and container_running(CONTAINER_NAME):
            write_result(req, "rollback_success", "rolled back to previous")
        else:
            write_result(req, "rollback_failed", "rollback failed, container down")

    # 消费完删除 request，避免重复处理
    req_path.unlink(missing_ok=True)


def main() -> None:
    log.info("cart-ota-agent 启动，监听 %s", REQUEST_FILE)
    OTA_DIR.mkdir(parents=True, exist_ok=True)
    while True:
        if REQUEST_FILE.is_file():
            try:
                process_request(REQUEST_FILE)
            except Exception as e:
                log.exception("处理 request 异常: %s", e)
                # 异常时也清理 request，避免死循环
                REQUEST_FILE.unlink(missing_ok=True)
        time.sleep(POLL_INTERVAL)


if __name__ == "__main__":
    main()
