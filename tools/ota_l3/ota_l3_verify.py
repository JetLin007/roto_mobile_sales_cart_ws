#!/usr/bin/env python3
# =============================================================================
# ota_l3_verify.py — OTA L3 宿主 agent + 容器重启全链路验证
# -----------------------------------------------------------------------------
# 验证链路（与 L2 的差异：agent 真实介入 + mock docker + 容器重启）：
#   /ota/command → service_ota_app → request.json
#     → cart_ota_agent.py → mock docker load/tag/restart → SIGTERM 杀 OTA 节点
#     → 新 OTA 节点进程起来 → report_last_result() 读 result.json → /ota/status 补报
# 用例：
#   A. 成功：agent 写 success → 重启 → 新进程看到 FINISHED/SUCCESS
#   B. 回滚：预置 previous，新镜像 revision 缺失或健康检查失败 → agent rollback → 新进程看到 ROLLBACK_SUCCESS
#   C. failed 不重启：镜像缺 revision → agent 写 failed 且不重启 → 旧进程继续运行
# =============================================================================
import hashlib
import json
import os
import shutil
import signal
import subprocess
import sys
import tempfile
import time
from pathlib import Path

import rclpy
from rclpy.node import Node
from robot_msgs.msg import OtaCommand, OtaStatus

APP = OtaCommand.TARGET_APP
TAR = OtaCommand.DELIVERY_TAR

STAGE_DOWNLOADING = OtaStatus.STAGE_DOWNLOADING
STAGE_INSTALLING = OtaStatus.STAGE_INSTALLING
STAGE_FINISHED = OtaStatus.STAGE_FINISHED
RESULT_NONE = OtaStatus.RESULT_NONE
RESULT_SUCCESS = OtaStatus.RESULT_SUCCESS
RESULT_FAILED = OtaStatus.RESULT_FAILED
RESULT_ROLLBACK_SUCCESS = OtaStatus.RESULT_ROLLBACK_SUCCESS
RESULT_ROLLBACK_FAILED = OtaStatus.RESULT_ROLLBACK_FAILED

# 为 ROS 进程设置可写日志目录（沙箱 HOME 只读）
os.environ.setdefault("ROS_LOG_DIR", "/tmp/ros_logs")

failures = []


def check(cond, name, detail=""):
    if cond:
        print("  [PASS] %s" % name)
    else:
        print("  [FAIL] %s %s" % (name, detail))
        failures.append(name)


class MockDockerState:
    """操作 mock docker 的 JSON state 文件。"""

    def __init__(self, state_dir: Path):
        self.path = state_dir / "images.json"
        if self.path.exists():
            self.path.unlink()

    def _read(self) -> dict:
        if not self.path.exists():
            return {"images": {}, "tags": {}, "container": {"pid": 0, "name": "roto-cart"}}
        return json.loads(self.path.read_text())

    def _write(self, state: dict) -> None:
        self.path.parent.mkdir(parents=True, exist_ok=True)
        self.path.write_text(json.dumps(state, indent=2))

    def set_container_pid(self, pid: int) -> None:
        s = self._read()
        s.setdefault("container", {"name": "roto-cart"})
        s["container"]["pid"] = pid
        self._write(s)

    def setup_previous(self, tar_path: str, revision: str, prev_tag: str) -> None:
        """直接在 state 里塞一个 previous 镜像（模拟已有 current 备份）。"""
        h = hashlib.sha256(Path(tar_path).read_bytes()).hexdigest()
        image_id = f"sha256:{h}"
        tag_key = prev_tag  # e.g. "roto-cart:previous"
        s = self._read()
        s["images"][image_id] = {
            "tags": [tag_key],
            "labels": {"org.opencontainers.image.revision": revision},
        }
        s["tags"][tag_key] = image_id
        self._write(s)

    def clear_all(self) -> None:
        self.path.unlink(missing_ok=True)


class OtaNodeProc:
    """OTA manager 节点子进程。提供 SIGTERM 事件捕获。"""

    def __init__(self, base_dir: Path, cart_id: str = "cart-l3"):
        self.base = base_dir
        self.work_dir = base_dir / "work"
        self.download_dir = base_dir / "cache"
        self.src_dir = base_dir / "src"
        for d in (self.work_dir, self.download_dir, self.src_dir):
            d.mkdir(parents=True, exist_ok=True)
        self.cart_id = cart_id
        self.proc: subprocess.Popen | None = None
        self.started_at: float = 0.0
        self.killed_at: float | None = None
        self._log_path = base_dir / f"node_{os.getpid()}.log"

    def exe(self) -> str:
        # 延迟导入 ament_index_python
        from ament_index_python.packages import get_package_prefix
        return os.path.join(
            get_package_prefix("service_ota_app"), "lib", "service_ota_app",
            "app_ota_manager_node")

    def start(self) -> None:
        # 每次 start 先清理可能存在的旧 result.json（agent 会重写）
        result_json = self.work_dir / "result.json"
        request_json = self.work_dir / "request.json"
        if request_json.exists():
            request_json.unlink()
        # result.json 保留（新节点需要读到 agent 的最终结果）

        env = os.environ.copy()
        env["ROS_LOG_DIR"] = os.environ.get("ROS_LOG_DIR", "/tmp/ros_logs")

        cmd = [
            self.exe(),
            "--ros-args",
            "-p", f"work_dir:={self.work_dir}",
            "-p", f"download_dir:={self.download_dir}",
            "-p", f"cart_id:={self.cart_id}",
        ]
        self._log_path = self.base / f"node_{self.started_at:.0f}.log"
        self._log_fh = open(self._log_path, "w")
        self.proc = subprocess.Popen(
            cmd, env=env, stdout=self._log_fh, stderr=subprocess.STDOUT)
        self.started_at = time.time()
        self.killed_at = None

    def wait_ready(self, harness: "Harness", timeout=15.0) -> bool:
        """通过 harness 节点的 DDS 拓扑确认 OTA 节点就绪。"""
        deadline = time.time() + timeout
        while time.time() < deadline:
            if not self.alive():
                time.sleep(0.2)
                continue
            # /ota/status 有发布者（OTA 节点）+ /ota/command 有订阅者（OTA 节点）
            # harness 自己也是 /ota/status 的订阅者，所以 >= 1 个发布者即可
            pubs = harness.count_publishers("/ota/status")
            subs = harness.count_subscribers("/ota/command")
            if pubs >= 1 and subs >= 1:
                return True
            time.sleep(0.3)
        return False

    def alive(self) -> bool:
        return self.proc is not None and self.proc.poll() is None

    def pid(self) -> int:
        return self.proc.pid if self.proc else 0

    def stop(self) -> None:
        if self.proc and self.proc.poll() is None:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=3)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait(timeout=2)
        try:
            self._log_fh.close()
        except Exception:
            pass


class AgentProc:
    """cart_ota_agent.py 进程。"""

    def __init__(self, base_dir: Path, state_dir: Path):
        self.base = base_dir
        self.state_dir = state_dir
        self.proc: subprocess.Popen | None = None

    def start(self, extra_env: dict | None = None) -> None:
        env = os.environ.copy()
        env["MOCK_DOCKER_STATE_DIR"] = str(self.state_dir)
        env["OTA_DIR"] = str(self.base / "work")
        env["OTA_HEALTH_WAIT"] = "5"  # 等 harness 拉起新进程
        env["OTA_POLL_INTERVAL"] = "0.2"  # 快速轮询
        if extra_env:
            env.update(extra_env)
        script = Path(__file__).resolve().parents[2] / "scripts" / "cart_ota_agent.py"
        self.proc = subprocess.Popen(
            [sys.executable, str(script)], env=env,
            stdout=open(self.base / "agent.log", "w"),
            stderr=subprocess.STDOUT)

    def alive(self) -> bool:
        return self.proc is not None and self.proc.poll() is None

    def stop(self) -> None:
        if self.proc and self.proc.poll() is None:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=3)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait(timeout=2)


class Harness(Node):
    def __init__(self):
        super().__init__("ota_l3_harness")
        self.statuses: list[OtaStatus] = []
        # 注意：发布者 QoS 必须和 OtaManagerNode 那边匹配（双方都是 RELIABLE）。
        # 旧命令被"重放"的真正原因不是 RELIABLE/BEST_EFFORT，而是 DDS 对端退订的延迟。
        # 解决：kill 之后显式等旧订阅者清零，再启新节点。
        from rclpy.qos import QoSProfile, QoSReliabilityPolicy
        qos = QoSProfile(depth=10, reliability=QoSReliabilityPolicy.RELIABLE)
        self.pub = self.create_publisher(OtaCommand, "/ota/command", qos)
        self.create_subscription(OtaStatus, "/ota/status", self._on_status, 10)

    def _on_status(self, msg):
        self.statuses.append(msg)

    def reset(self):
        """清空状态 + 重建 /ota/command 发布者（清空 DDS History，避免旧指令重放）。"""
        self.statuses = []
        # 销毁旧 pub → 清空 History 队列里残留的旧 OtaCommand
        self.pub = None
        for _ in range(3):
            rclpy.spin_once(self, timeout_sec=0.02)
        # 重建 pub（fresh History）
        from rclpy.qos import QoSProfile, QoSReliabilityPolicy
        qos = QoSProfile(depth=10, reliability=QoSReliabilityPolicy.RELIABLE)
        self.pub = self.create_publisher(OtaCommand, "/ota/command", qos)
        for _ in range(3):
            rclpy.spin_once(self, timeout_sec=0.02)

    def wait_until(self, pred, timeout_sec):
        deadline = time.time() + timeout_sec
        while time.time() < deadline:
            rclpy.spin_once(self, timeout_sec=0.05)
            if pred(self.statuses):
                return True
        return False

    def subscribers_gone(self, topic: str) -> bool:
        """旧节点被杀后，DDS 可能还留一会儿。轮询直到订阅数为 0。"""
        deadline = time.time() + 10.0
        while time.time() < deadline:
            if self.count_subscribers(topic) == 0:
                return True
            time.sleep(0.1)
        return False


def make_payload_image(src_dir: Path, name: str, content: bytes, revision: str) -> tuple[Path, str]:
    path = src_dir / name
    path.write_bytes(content)
    sha = hashlib.sha256(content).hexdigest()
    # 把 revision 记录到文件名后缀（mock docker 不需要额外 env 来识别）
    return path, sha


def make_command(task_id, module, url, sha256, version="v9.9.9-l3", soc=0):
    cmd = OtaCommand()
    now_ms = int(time.time() * 1000)
    cmd.header.msg_id = task_id + "-msg"
    cmd.header.timestamp_ms = now_ms
    cmd.header.protocol_version = "1.3"
    cmd.header.cart_id = "cart-l3"
    cmd.ota_task_id = task_id
    cmd.target_module = module
    cmd.target_version = version
    cmd.url = url
    cmd.sha256 = sha256
    cmd.signature = ""
    cmd.delivery = TAR
    cmd.min_battery_soc = soc
    return cmd


def docker_bin_path(state_dir: Path) -> str:
    """返回一个 PATH 目录，其中 docker 指向我们的 mock。"""
    return str(Path(__file__).resolve().parent)  # mock_docker.py 就在本目录


def build_path(env_path: str, state_dir: Path) -> str:
    mock_dir = Path(__file__).resolve().parent
    return f"{mock_dir}:{env_path}"


def run_mock_docker(state_dir: Path, args: list[str]) -> subprocess.CompletedProcess:
    env = os.environ.copy()
    env["MOCK_DOCKER_STATE_DIR"] = str(state_dir)
    env["PATH"] = build_path(env.get("PATH", ""), state_dir)
    mock_bin = str(Path(__file__).resolve().parent / "mock_docker.py")
    return subprocess.run(
        [sys.executable, mock_bin] + args, capture_output=True, text=True, env=env)


# =============================================================================
# Case A: Happy path — 完整 node→agent→docker→restart→补报 链路
# =============================================================================
def case_happy(harness: Harness) -> None:
    print("[Case A] Happy path: node→agent→restart→补报 FINISHED/SUCCESS")
    base = Path(tempfile.mkdtemp(prefix="ota_l3_A_"))
    state_dir = base / "mockdocker"
    state_dir.mkdir()

    # 把 mock_docker.py 链接成可执行的 "docker"
    bin_dir = base / "bin"
    bin_dir.mkdir()
    mock_bin = Path(__file__).resolve().parent / "mock_docker.py"
    (bin_dir / "docker").write_text(f"#!{sys.executable}\nimport os,sys,runpy\nos.environ['MOCK_DOCKER_STATE_DIR']='{state_dir}'\nsys.argv=['docker']+sys.argv[1:]\nrunpy.run_path(r'{mock_bin}', run_name='__main__')\n")
    os.chmod(bin_dir / "docker", 0o755)
    original_path = os.environ.get("PATH", "")
    os.environ["PATH"] = f"{bin_dir}:{original_path}"

    mock_state = MockDockerState(state_dir)
    agent = AgentProc(base, state_dir)
    node = OtaNodeProc(base)

    try:
        # 1) 起 OTA 节点（PID=N1）
        node.start()
        check(node.wait_ready(harness), "OTA 节点就绪 (进程 PID=%d)" % node.pid())
        mock_state.set_container_pid(node.pid())

        harness.reset()

        # 2) 发 OTA 指令
        tar_path, sha = make_payload_image(
            node.src_dir, "roto-cart:1.3.0-arm64.tar",
            b"fake docker image content for l3 happy",
            revision="rev-A1")
        harness.pub.publish(make_command(
            "L3-HAPPY", APP, "file://" + str(tar_path), sha,
            version="v1.3.0-arm64", soc=30))

        # 3) 等 request.json（容器内 OtaCore 写完握手）
        ok = harness.wait_until(
            lambda _: (base / "work" / "request.json").exists(), 15.0)
        check(ok, "request.json 已写入")
        req_json = json.loads((base / "work" / "request.json").read_text())
        check(req_json.get("ota_task_id") == "L3-HAPPY", "request.ota_task_id")
        check(req_json.get("delivery") == "tar", "request.delivery=tar")
        check(req_json.get("min_battery_soc") == 30, "request.min_battery_soc=30")

        # 4) 起 agent
        agent.start()
        check(agent.alive(), "agent 进程启动")

        # 5) 等待 agent 第一次 SIGTERM（mock docker restart）
        deadline = time.time() + 15.0
        while time.time() < deadline:
            if not node.alive():
                break
            time.sleep(0.2)
        check(not node.alive(), "agent 已 SIGTERM 杀死旧 OTA 节点（模拟 docker restart）")

        # **关键**：agent 的健康检查窗口只有 5s，我们必须在此窗口内拉起新节点，
        # 不等 DDS 退订（新节点订阅新 PID，agent 会立刻看到 alive）。
        # DDS 重放旧命令的风险靠 subscribers_gone 后面等 + RELIABLE 的旧消息会在
        # agent restart 清 PID 后被丢弃（我们在新节点 ready 后再等一次并 reset）。
        node.stop()  # 确保干净回收
        harness.reset()  # 冲刷旧 statuses（此时可能已看到 DOWNLOADING 等中间态）
        node.start()
        check(node.wait_ready(harness, timeout=10.0), "agent 重启后新节点就绪（5s 健康窗口内）")
        mock_state.set_container_pid(node.pid())

        # 等 agent 健康检查过 → 结果确定（它现在应该已看到新 PID alive，写 success）
        time.sleep(1.0)

        result_path = base / "work" / "result.json"
        check(result_path.exists(), "result.json 已写好")
        result = json.loads(result_path.read_text())
        check(result.get("status") == "success",
              "result.status=success", f"got {result.get('status')}")
        check(result.get("ota_task_id") == "L3-HAPPY", "result.ota_task_id")

        # 6) 新节点 report_last_result() 补报 FINISHED/SUCCESS
        ok = harness.wait_until(
            lambda st: any(s.result == RESULT_SUCCESS for s in st), 10.0)
        check(ok, "新节点补报 result=SUCCESS")
        final = harness.statuses[-1] if harness.statuses else None
        check(final is not None, "有最终 OtaStatus")
        check(final.stage == STAGE_FINISHED,
              "final.stage=FINISHED", f"got {final.stage}")
        check(final.progress == 100,
              "final.progress=100", f"got {final.progress}")
        check(final.ota_task_id == "L3-HAPPY", "final.ota_task_id")
        check(final.target_version == "v1.3.0-arm64", "final.target_version")
        check(final.header.protocol_version == "1.3", "protocol_version=1.3")

    finally:
        agent.stop()
        node.stop()
        os.environ["PATH"] = original_path
        if failures:
            print("---- agent.log ----\n%s" % (base / "agent.log").read_text())
        shutil.rmtree(base, ignore_errors=True)


# =============================================================================
# Case B: 回滚 — health check 失败 → agent rollback_success → 新进程补报
# =============================================================================
def case_rollback(harness: Harness) -> None:
    print("[Case B] Rollback: health 失败 → 回滚 → 新进程 ROLLBACK_SUCCESS")
    base = Path(tempfile.mkdtemp(prefix="ota_l3_B_"))
    state_dir = base / "mockdocker"
    state_dir.mkdir()

    bin_dir = base / "bin"
    bin_dir.mkdir()
    mock_bin = Path(__file__).resolve().parent / "mock_docker.py"
    (bin_dir / "docker").write_text(
        f"#!{sys.executable}\nimport os,sys,runpy\n"
        f"os.environ['MOCK_DOCKER_STATE_DIR']='{state_dir}'\n"
        f"sys.argv=['docker']+sys.argv[1:]\n"
        f"runpy.run_path(r'{mock_bin}', run_name='__main__')\n")
    os.chmod(bin_dir / "docker", 0o755)
    original_path = os.environ.get("PATH", "")
    os.environ["PATH"] = f"{bin_dir}:{original_path}"

    mock_state = MockDockerState(state_dir)
    agent = AgentProc(base, state_dir)
    node = OtaNodeProc(base)

    try:
        # 0) 预置：既有 current（让 had_previous=True）也有 previous（让回滚时有底可退）
        prev_tar = node.src_dir / "prev-image.tar"
        prev_tar.write_bytes(b"previous image content")
        cur_tar = node.src_dir / "cur-image.tar"
        cur_tar.write_bytes(b"current image content")
        mock_state.setup_previous(str(prev_tar), revision="rev-PREV",
                                  prev_tag="roto-cart:previous")
        mock_state.setup_previous(str(cur_tar), revision="rev-CURR",
                                  prev_tag="roto-cart:current")

        # 1) 起 OTA 节点
        node.start()
        check(node.wait_ready(harness), "OTA 节点就绪")
        mock_state.set_container_pid(node.pid())
        harness.reset()

        # 2) 发 OTA 指令
        tar_path, sha = make_payload_image(
            node.src_dir, "roto-cart:1.4.0-new.tar",
            b"brand new image that will be OTA'd",
            revision="rev-NEW")
        harness.pub.publish(make_command(
            "L3-ROLLBACK", APP, "file://" + str(tar_path), sha,
            version="v1.4.0-new", soc=30))

        # 3) 等 request.json
        ok = harness.wait_until(
            lambda _: (base / "work" / "request.json").exists(), 15.0)
        check(ok, "request.json 已写入")

        # 4) 起 agent
        agent.start()

        # 5) 等待 agent 第一次 SIGTERM（success 路径 restart）
        deadline = time.time() + 15.0
        while time.time() < deadline:
            if not node.alive():
                break
            time.sleep(0.2)
        check(not node.alive(), "agent 第一次 SIGTERM（success 路径）")

        # **关键**：不拉起新节点！让 agent 的 _wait_healthy(5s) 超时 → 进入回滚路径
        # 等待 result.json 里出现 rollback_success（agent 在 rollback path 写好的）
        result_path = base / "work" / "result.json"
        ok = False
        deadline = time.time() + 15.0
        while time.time() < deadline:
            if result_path.exists():
                try:
                    r = json.loads(result_path.read_text())
                    if r.get("status") == "rollback_success":
                        ok = True
                        break
                except Exception:
                    pass
            time.sleep(0.3)
        check(ok,
              "agent rollback 完成（result.json=rollback_success）",
              f"agent.log tail: {(base/'agent.log').read_text()[-500:]}")

        # agent rollback 里又做了一次 docker restart（PID=0），同样要等 DDS 退订
        harness.subscribers_gone("/ota/command")
        harness.subscribers_gone("/ota/status")

        # 6) 拉起新节点（agent 已经回滚完毕，result.json 已稳定在 rollback_success）
        node.stop()
        harness.reset()  # 冲刷 DDS + 清空 statuses
        node.start()
        check(node.wait_ready(harness, timeout=10.0), "回滚后新 OTA 节点就绪")
        mock_state.set_container_pid(node.pid())

        result = json.loads(result_path.read_text())
        check(result.get("status") == "rollback_success",
              "result.status=rollback_success", f"got {result.get('status')}")

        # 7) 等新节点补报 ROLLBACK_SUCCESS
        ok = harness.wait_until(
            lambda st: any(s.result == RESULT_ROLLBACK_SUCCESS for s in st), 10.0)
        check(ok, "新节点补报 result=ROLLBACK_SUCCESS")
        final = harness.statuses[-1] if harness.statuses else None
        check(final is not None, "有最终 OtaStatus")
        check(final.stage == STAGE_FINISHED, "final.stage=FINISHED",
              f"got {final.stage}")

    finally:
        agent.stop()
        node.stop()
        os.environ["PATH"] = original_path
        if failures:
            print("---- agent.log ----\n%s" % (base / "agent.log").read_text())
        shutil.rmtree(base, ignore_errors=True)


# =============================================================================
# Case C: failed — 镜像缺 revision label，agent 拒绝，不重启
# =============================================================================
def case_failed(harness: Harness) -> None:
    print("[Case C] Failed: 缺 revision → agent 拒绝安装，不重启容器")
    base = Path(tempfile.mkdtemp(prefix="ota_l3_C_"))
    state_dir = base / "mockdocker"
    state_dir.mkdir()

    bin_dir = base / "bin"
    bin_dir.mkdir()
    mock_bin = Path(__file__).resolve().parent / "mock_docker.py"
    (bin_dir / "docker").write_text(
        f"#!{sys.executable}\nimport os,sys,runpy\n"
        f"os.environ['MOCK_DOCKER_STATE_DIR']='{state_dir}'\n"
        f"sys.argv=['docker']+sys.argv[1:]\n"
        f"runpy.run_path(r'{mock_bin}', run_name='__main__')\n")
    os.chmod(bin_dir / "docker", 0o755)
    original_path = os.environ.get("PATH", "")
    os.environ["PATH"] = f"{bin_dir}:{original_path}"

    mock_state = MockDockerState(state_dir)
    agent = AgentProc(base, state_dir)
    node = OtaNodeProc(base)

    try:
        node.start()
        check(node.wait_ready(harness), "OTA 节点就绪")
        mock_state.set_container_pid(node.pid())
        harness.reset()

        tar_path, sha = make_payload_image(
            node.src_dir, "bad-image.tar",
            b"no revision label in docker image",
            revision="")  # revision 空 → mock load 不写入 label

        harness.pub.publish(make_command(
            "L3-FAILED", APP, "file://" + str(tar_path), sha,
            version="v0.0.0-bad", soc=0))

        ok = harness.wait_until(
            lambda _: (base / "work" / "request.json").exists(), 15.0)
        check(ok, "request.json 已写入")

        # 关键：让 mock docker 的 load 返回空 revision，触发 agent 的 revision 校验失败路径
        agent.start(extra_env={"MOCK_DOCKER_NO_REVISION": "1"})

        # 等 agent 写完 result.json（写 failed，且不重启）
        result_path = base / "work" / "result.json"
        ok = False
        deadline = time.time() + 10.0
        while time.time() < deadline:
            if result_path.exists():
                ok = True
                break
            time.sleep(0.2)
        check(ok, "agent 已写 result.json")

        result = json.loads(result_path.read_text())
        check(result.get("status") == "failed",
              "result.status=failed", f"got {result.get('status')}")
        check("OCI label revision" in result.get("message", ""),
              "失败原因包含 revision 提示", result.get("message"))

        # 关键：agent 不应重启容器，所以 node 应该还活着
        check(node.alive(), "容器未被重启（失败路径只写 result.json）")

    finally:
        agent.stop()
        node.stop()
        os.environ["PATH"] = original_path
        if failures:
            print("---- agent.log ----\n%s" % (base / "agent.log").read_text())
        shutil.rmtree(base, ignore_errors=True)


def tail_node_log(base: Path, n: int = 15) -> str:
    """返回最新的 node_*.log 尾部关键行。"""
    logs = sorted(base.glob("node_*.log"))
    if not logs:
        return "<no node logs>"
    latest = logs[-1]
    try:
        lines = latest.read_text(errors="replace").splitlines()
        return "\n".join(
            l for l in lines if any(kw in l for kw in (
                "report last", "started", "publish", "OTA ", "ERROR", "WARN", "INFO"))
        )[-12:]
    except Exception as e:
        return f"<read error: {e}>"


def main():
    rclpy.init()
    harness = Harness()
    try:
        case_happy(harness)
        case_rollback(harness)
        case_failed(harness)
    finally:
        harness.destroy_node()
        rclpy.shutdown()

    print()
    if failures:
        print("L3 验证失败：%d 项" % len(failures))
        for name in failures:
            print("  - " + name)
        return 1
    print("L3 验证通过：OTA 全链路（node→agent→mock docker→restart→补报）符合预期")
    return 0


if __name__ == "__main__":
    sys.exit(main())
