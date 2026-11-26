#!/usr/bin/env python3
# =============================================================================
# mock_docker.py — 模拟 docker CLI，供 L3 验证使用
# -----------------------------------------------------------------------------
# 在没有 Docker 的开发环境下，让 cart_ota_agent.py 的 subprocess.run(["docker", ...])
# 调用落到这里。与真实 docker 不同点：
#   * 镜像 registry 与容器状态存在 JSON 文件（env MOCK_DOCKER_STATE_DIR）
#   * CONTAINER_NAME 指向一个 OS 进程 PID，restart 会发送 SIGTERM 杀死它
#     （测试 harness 在被杀死后拉起新进程，模拟 docker restart 语义）
#
# 覆盖的子命令：
#   docker load -i <file>                 → 读文件 sha256 作为 image_id
#                                           输出 "Loaded image: <image_id>:<sha8>"
#   docker tag <src> <dst>                → 建立别名
#   docker image inspect <image>          → 检查 image 是否在 registry
#   docker inspect -f "{{.State.Running}}" <container> → 检查进程是否存活
#   docker inspect -f "{{index .Config.Labels \"org.opencontainers.image.revision\"}}" <image>
#                                           → 返回镜像 label
#   docker restart <container>            → SIGTERM 杀死进程（harness 负责拉起新进程）
# =============================================================================
import hashlib
import json
import os
import signal
import subprocess
import sys
import time
from pathlib import Path

STATE_DIR = Path(os.getenv("MOCK_DOCKER_STATE_DIR", "/tmp/mock_docker"))
IMAGES_FILE = STATE_DIR / "images.json"
CONTAINER_FILE = STATE_DIR / "container.json"

CONTAINER_PID = int(os.getenv("OTA_CONTAINER", "0") or "0")
NO_REVISION = os.getenv("MOCK_DOCKER_NO_REVISION", "") == "1"


def load_state() -> dict:
    if not IMAGES_FILE.is_file():
        return {"images": {}, "tags": {}, "container": {"pid": CONTAINER_PID, "name": "roto-cart"}}
    state = json.loads(IMAGES_FILE.read_text())
    state.setdefault("container", {"pid": CONTAINER_PID, "name": "roto-cart"})
    return state


def save_state(state: dict) -> None:
    STATE_DIR.mkdir(parents=True, exist_ok=True)
    IMAGES_FILE.write_text(json.dumps(state, indent=2))


def _current_container_pid(state: dict, ref: str) -> int:
    """从 state / env / ref 解析容器 PID。"""
    if ref.isdigit():
        return int(ref)
    return state.get("container", {}).get("pid", CONTAINER_PID)


def resolve_image(state: dict, ref: str) -> str | None:
    """把 tag 别名解析到 image_id。"""
    if ref in state["images"]:
        return ref
    return state["tags"].get(ref)


def handle_load(args: list[str]) -> int:
    if "-i" not in args:
        print("mock docker: load needs -i", file=sys.stderr)
        return 1
    tar_idx = args.index("-i") + 1
    tar_path = args[tar_idx]
    h = hashlib.sha256()
    with open(tar_path, "rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    sha_full = h.hexdigest()
    sha8 = sha_full[:8]
    image_id = f"sha256:{sha_full}"

    # 尝试从 tar 文件名里提取一个更像样的 tag（形如 roto-cart:1.3.0-arm64）
    base = Path(tar_path).stem.replace(".tar", "").replace(".oci", "")
    if NO_REVISION:
        label_rev = ""  # 强制空 revision，用于模拟 build_image.sh 漏打 label 的故障场景
    else:
        label_rev = os.getenv("MOCK_DOCKER_IMAGE_REVISION", "")
        label_rev_override = os.getenv("MOCK_DOCKER_IMAGE_REVISION_" + sha8.upper(), "")
        label_rev = label_rev_override or label_rev or sha8

    state = load_state()
    state["images"][image_id] = {
        "tags": [f"{base}:{sha8}", f"{base}:latest"],
        "labels": {"org.opencontainers.image.revision": label_rev},
    }
    for tag in state["images"][image_id]["tags"]:
        state["tags"][tag] = image_id
    save_state(state)

    print(f"Loaded image: {base}:{sha8}")
    print(f"SHA256:{sha_full}")
    return 0


def handle_tag(args: list[str]) -> int:
    if len(args) < 2:
        return 1
    src, dst = args[0], args[1]
    state = load_state()
    image_id = resolve_image(state, src)
    if image_id is None:
        # 如果 src 是个还没登记过的本地镜像（比如我们刚 load 的），尝试补登记
        image_id = src
        if image_id not in state["images"]:
            state["images"][image_id] = {"tags": [], "labels": {}}
    state["images"][image_id]["tags"].append(dst)
    state["tags"][dst] = image_id
    save_state(state)
    return 0


def handle_image_inspect(args: list[str]) -> int:
    if not args:
        return 1
    ref = args[-1]
    state = load_state()
    image_id = resolve_image(state, ref)
    if image_id is None:
        print(f"Error: No such image: {ref}", file=sys.stderr)
        return 1
    # 输出简化的 JSON
    img = state["images"][image_id]
    tags = img["tags"]
    print(json.dumps([
        {
            "Id": image_id,
            "RepoTags": tags,
            "Config": {"Labels": img.get("labels", {})},
        }
    ]))
    return 0


def handle_inspect(args: list[str]) -> int:
    if not args:
        return 1
    # docker inspect [-f "{{...}}"] <ref>
    ref = args[-1]
    f_arg = None
    if "-f" in args:
        f_arg = args[args.index("-f") + 1]

    # 检查是不是容器引用
    state = load_state()
    if ref.isdigit() or ref == state.get("container", {}).get("name", "roto-cart"):
        pid = _current_container_pid(state, ref)
        if f_arg and ".State.Running" in f_arg:
            running = pid > 0 and _pid_alive(pid)
            print("true" if running else "false")
            return 0
        # 容器不存在或不查询 running → 返回空
        return 1

    # 镜像
    state = load_state()
    image_id = resolve_image(state, ref)
    if image_id is None:
        print(f"Error: No such object: {ref}", file=sys.stderr)
        return 1
    img = state["images"][image_id]

    if f_arg is None:
        # 完整 JSON
        print(json.dumps([{
            "Id": image_id,
            "RepoTags": img["tags"],
            "Config": {"Labels": img.get("labels", {})},
        }]))
        return 0

    # 解析 -f 模板（只支持两种我们需要的）
    if "{{.State.Running}}" in f_arg:
        print("false")  # 镜像没有 running 状态
        return 1
    if 'org.opencontainers.image.revision' in f_arg:
        print(img.get("labels", {}).get("org.opencontainers.image.revision", ""))
        return 0

    print(f"unsupported -f template: {f_arg}", file=sys.stderr)
    return 1


def handle_restart(args: list[str]) -> int:
    if not args:
        return 1
    ref = args[-1]
    state = load_state()
    pid = _current_container_pid(state, ref)
    if pid <= 0:
        # 真实 docker 对不存在的容器会尝试 start（restart = kill+start）。
        # 我们只做 kill，没有 start 的语义——所以这里模拟"已接受 restart 请求"。
        # harness 会在稍后自行拉起新进程并更新 state 里的 PID。
        print(f"mock docker: 无活跃进程，restart 请求已接受 (pid=0)")
        return 0
    try:
        os.kill(pid, signal.SIGTERM)
    except ProcessLookupError:
        print(f"mock docker: 进程 {pid} 不存在，已视为退出")
    else:
        deadline = time.time() + 5.0
        while time.time() < deadline:
            try:
                os.kill(pid, 0)
            except ProcessLookupError:
                break
            except PermissionError:
                break
            time.sleep(0.05)
        else:
            try:
                os.kill(pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
    # 清掉 PID — "容器已退出，新进程尚未拉起"
    state.setdefault("container", {})["pid"] = 0
    save_state(state)
    print(f"mock docker: 进程 {pid} 已退出")
    return 0


def handle_pull(args: list[str]) -> int:
    if not args:
        return 1
    ref = args[-1]
    # pull 模式在 L3 不重点测，mock 只登记一下 registry 里有这个 ref
    state = load_state()
    image_id = f"sha256:pull-{hash(ref) % 1_000_000:06d}"
    if image_id not in state["images"]:
        state["images"][image_id] = {"tags": [ref], "labels": {"org.opencontainers.image.revision": ref.split(":")[-1]}}
        state["tags"][ref] = image_id
        save_state(state)
    print(f"Using default tag: latest\nDownloaded newer image for {ref}")
    return 0


def _pid_alive(pid: int) -> bool:
    try:
        os.kill(pid, 0)
        return True
    except (ProcessLookupError, PermissionError):
        return False


SUB_COMMANDS = {
    "load": handle_load,
    "tag": handle_tag,
    "restart": handle_restart,
    "pull": handle_pull,
}


def main(argv: list[str]) -> int:
    if len(argv) < 2:
        print("mock docker: 用法 docker <subcommand> [args...]", file=sys.stderr)
        return 1
    sub = argv[1]
    rest = argv[2:]

    if sub == "image" and rest:
        sub = rest[0]
        rest = rest[1:]

    if sub in SUB_COMMANDS:
        return SUB_COMMANDS[sub](rest)
    if sub == "inspect":
        return handle_inspect(rest)

    print(f"mock docker: 未实现子命令 {sub}", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
