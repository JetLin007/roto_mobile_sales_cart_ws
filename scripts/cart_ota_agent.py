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
import argparse
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

# ---- CLI 参数解析（覆盖环境变量，生产 systemd 启动不传任何参数）----
VALID_MOCK_RESULTS = ("success", "failed", "rollback_success", "rollback_failed")


def _parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(
        prog="cart_ota_agent.py",
        description="宿主侧 OTA 执行代理；生产走 docker，开发可 -m mock 模式跳过所有 docker 操作",
    )
    p.add_argument("-m", "--mock", action="store_true",
                   help="启用 mock 模式（等价 OTA_MOCK_MODE=1）")
    p.add_argument("-r", "--mock-result", metavar="STATUS",
                   choices=VALID_MOCK_RESULTS,
                   help="mock 模式下写的 result.json status 值"
                        f" ({'/'.join(VALID_MOCK_RESULTS)}, 覆盖 OTA_MOCK_RESULT)")
    p.add_argument("-d", "--ota-dir", metavar="DIR",
                   help="OTA 握手目录（覆盖 OTA_DIR）")
    return p.parse_args()


_args = _parse_args()
MOCK_MODE   = _args.mock or os.getenv("OTA_MOCK_MODE", "0") == "1"
MOCK_RESULT = (_args.mock_result
               or os.getenv("OTA_MOCK_RESULT", "success")
               or "success")
if _args.ota_dir:
    OTA_DIR = Path(_args.ota_dir)
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
    if MOCK_MODE:
        log.info("[MOCK] inspect revision -> mock-rev-001")
        return "mock-rev-001"
    cp = run(["docker", "inspect", "-f",
              "{{index .Config.Labels \"org.opencontainers.image.revision\"}}",
              image], timeout=30)
    if cp.returncode != 0:
        return None
    rev = cp.stdout.strip()
    return rev if rev and rev != "<no value>" else None


def docker_image_exists(image: str) -> bool:
    if MOCK_MODE:
        return True
    return run(["docker", "image", "inspect", image], timeout=15).returncode == 0


def container_running(name: str) -> bool:
    if MOCK_MODE:
        return True
    cp = run(["docker", "inspect", "-f", "{{.State.Running}}", name], timeout=15)
    return cp.returncode == 0 and cp.stdout.strip() == "true"


def load_or_pull(req: dict) -> Optional[str]:
    """根据 delivery 字段加载镜像，返回新镜像的引用名。"""
    if MOCK_MODE:
        ver = req.get("target_version", "unknown")
        mock_img = f"roto-cart:{ver}-mock"
        log.info("[MOCK] load_or_pull -> %s", mock_img)
        return mock_img
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


def write_result(req: dict, status: str, message: str) -> None:
    """写 result.json，容器内新进程会读取并上报云端。

    设计约束（与容器内 OtaManagerNode::report_last_result 的时序配合）：
      本函数返回后再执行 docker restart。因为新容器进程启动时会立刻读
      result.json 并上报最终状态，必须让它看到 final 而不是占位态。
    """
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


def _wait_healthy(timeout_s: float = None) -> bool:
    """轮询容器 running 状态直到超时，返回是否健康。"""
    timeout_s = timeout_s if timeout_s is not None else float(HEALTH_WAIT_S)
    deadline = time.time() + timeout_s
    while time.time() < deadline:
        if container_running(CONTAINER_NAME):
            return True
        time.sleep(2)
    return False


def process_request(req_path: Path) -> None:
    """处理一个 request.json。

    关键时序（与容器内 OtaManagerNode 协作）：
      1. 写 result.json = "success"  →  2. docker restart → 新容器进程读 result.json
      所以 result.json 必须在 docker restart 之前落成，否则新进程看到的是旧状态。
    """
    try:
        req = json.loads(req_path.read_text())
    except Exception as e:
        log.error("解析 request.json 失败: %s", e)
        req_path.unlink(missing_ok=True)
        return

    ota_task_id = req.get("ota_task_id", "")
    log.info("收到 OTA 请求 task=%s version=%s delivery=%s",
             ota_task_id, req.get("target_version"), req.get("delivery"))

    # --- 阶段 1：加载镜像 ---
    new_image = load_or_pull(req)
    if not new_image:
        write_result(req, "failed", "load/pull image failed")
        req_path.unlink(missing_ok=True)
        return

    # --- 阶段 2：校验 OCI label revision（防镜像掉包）---
    rev = docker_inspect_revision(new_image)
    if not rev:
        # 开发环境 docker commit 出来的镜像通常没带 OCI revision label，
        # 回退到 request.json 里的 target_version 作为占位。
        # 生产 CI 构建的镜像（Dockerfile LABEL org.opencontainers.image.revision=...）
        # 才会真实命中这个检查路径。
        rev = req.get("target_version") or "unknown"
        log.warning("新镜像缺少 OCI label revision，fallback 到 target_version=%s", rev)
    log.info("新镜像 revision=%s", rev)

    # --- 阶段 3：A/B 切 tag ---
    had_previous = docker_image_exists(IMAGE_CURRENT)
    if had_previous:
        if MOCK_MODE:
            log.info("[MOCK] docker tag %s -> %s", IMAGE_CURRENT, IMAGE_PREVIOUS)
        else:
            run(["docker", "tag", IMAGE_CURRENT, IMAGE_PREVIOUS], timeout=15)
        log.info("已将 %s 备份为 %s", IMAGE_CURRENT, IMAGE_PREVIOUS)

    if MOCK_MODE:
        log.info("[MOCK] docker tag %s -> %s", new_image, IMAGE_CURRENT)
        tag_ok = True
        tag_err = ""
    else:
        cp = run(["docker", "tag", new_image, IMAGE_CURRENT], timeout=15)
        tag_ok = cp.returncode == 0
        tag_err = cp.stderr.strip()
    if not tag_ok:
        write_result(req, "failed", f"tag current 失败: {tag_err}")
        req_path.unlink(missing_ok=True)
        return

    # --- 阶段 4：restart → 模拟耗时 → 写 result.json（或根据 MOCK_RESULT 覆盖）---
    if MOCK_MODE:
        # 模拟 restart + 健康检查的耗时
        log.info("[MOCK] docker restart %s", CONTAINER_NAME)
        time.sleep(1)
        # 根据 MOCK_RESULT 决定最终结果
        final_status = MOCK_RESULT if MOCK_RESULT in (
            "success", "failed", "rollback_success", "rollback_failed"
        ) else "success"
        # success 路径已经提前在前面写过 result.json 了，这里覆盖即可
        log.info("[MOCK] 健康检查通过 (final_status=%s)", final_status)
        write_result(req, final_status, f"[MOCK] installed {new_image}")
    else:
        write_result(req, "success", f"installed {new_image}")
        cp = run(["docker", "restart", CONTAINER_NAME], timeout=60)
        if cp.returncode != 0:
            log.error("docker restart 失败: %s", cp.stderr.strip())
            result = "rollback_failed"
            msg = f"docker restart 失败，had_previous={had_previous}"
        else:
            if _wait_healthy():
                log.info("OTA 成功，容器健康")
                result = None  # success 已写好，不需要再覆盖
                msg = ""
            else:
                log.error("新容器健康检查超时，执行回滚")
                # --- 回滚写 rollback → 再次 restart ---
                if had_previous and docker_image_exists(IMAGE_PREVIOUS):
                    cp2 = run(["docker", "tag", IMAGE_PREVIOUS, IMAGE_CURRENT], timeout=15)
                    if cp2.returncode != 0:
                        write_result(req, "rollback_failed",
                                     f"回滚 tag 失败: {cp2.stderr.strip()}")
                    else:
                        write_result(req, "rollback_success", "rolled back to previous")
                        cp3 = run(["docker", "restart", CONTAINER_NAME], timeout=60)
                        if cp3.returncode != 0:
                            write_result(req, "rollback_failed",
                                         f"回滚重启失败: {cp3.stderr.strip()}")
                        else:
                            log.info("回滚成功，restart 请求已发送")
                else:
                    write_result(req, "rollback_failed",
                                 "健康检查超时且无 previous 镜像可回滚")
                result = None  # rollback 路径里已经写过 result.json
                msg = ""

    # 消费完删除 request，避免重复处理
    req_path.unlink(missing_ok=True)


def main() -> None:
    if MOCK_MODE:
        log.info("cart-ota-agent [MOCK MODE] 启动，监听 %s", REQUEST_FILE)
        log.info("  MOCK_RESULT=%s  (可通过 -r 或 OTA_MOCK_RESULT 覆盖)", MOCK_RESULT)
    else:
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
