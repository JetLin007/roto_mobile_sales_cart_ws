#!/usr/bin/env python3
# =============================================================================
# ota_l2_verify.py — OTA 功能 L2 验证（ROS 节点联调）
# -----------------------------------------------------------------------------
# 验证链路：/ota/command (OtaCommand) -> service_ota_app -> /ota/status (OtaStatus)
# 不依赖 MQTT Broker：本脚本扮演 cloud_comm 转发后的角色，直接下发 ROS 消息。
# 用例：
#   A. APP 正常升级：阶段序列 DOWNLOADING->VERIFYING->INSTALLING，request.json 正确
#   B. sha256 不匹配：终态 FAILED，不产生 request.json，下载文件被清理
#   C. 连续两个任务：验证工作线程复用不崩溃（第二次任务仍能推进）
#   D. CHASSIS 任务：target_module 透传、request.json 走 tar + url 落盘
# 运行（先 source ROS 与本工程 install/setup.bash）：
#   python3 tools/ota_l2/ota_l2_verify.py
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

import rclpy
from ament_index_python.packages import get_package_prefix
from rclpy.node import Node
from robot_msgs.msg import OtaCommand, OtaStatus

APP = OtaCommand.TARGET_APP
CHASSIS = OtaCommand.TARGET_CHASSIS
TAR = OtaCommand.DELIVERY_TAR

STAGE_DOWNLOADING = OtaStatus.STAGE_DOWNLOADING
STAGE_VERIFYING = OtaStatus.STAGE_VERIFYING
STAGE_INSTALLING = OtaStatus.STAGE_INSTALLING
RESULT_NONE = OtaStatus.RESULT_NONE
RESULT_FAILED = OtaStatus.RESULT_FAILED


class Harness(Node):
    def __init__(self):
        super().__init__("ota_l2_harness")
        self.statuses = []
        self.pub = self.create_publisher(OtaCommand, "/ota/command", 10)
        self.create_subscription(OtaStatus, "/ota/status", self._on_status, 10)

    def _on_status(self, msg):
        self.statuses.append(msg)

    def reset(self):
        self.statuses = []

    def pump(self, seconds):
        deadline = time.time() + seconds
        while time.time() < deadline:
            rclpy.spin_once(self, timeout_sec=0.05)

    def wait_until(self, pred, timeout_sec):
        deadline = time.time() + timeout_sec
        while time.time() < deadline:
            rclpy.spin_once(self, timeout_sec=0.05)
            if pred(self.statuses):
                return True
        return False

    def wait_ready(self, timeout_sec=15.0):
        deadline = time.time() + timeout_sec
        while time.time() < deadline:
            rclpy.spin_once(self, timeout_sec=0.05)
            if self.count_publishers("/ota/status") > 0 and \
               self.count_subscribers("/ota/command") > 0:
                return True
        return False


class OtaNodeProc:
    """以子进程方式启动 app_ota_manager_node，测试结束确保回收。"""

    def __init__(self, base_dir, cart_id="cart-l2"):
        self.work_dir = os.path.join(base_dir, "work")
        self.download_dir = os.path.join(base_dir, "cache")
        self.src_dir = os.path.join(base_dir, "src")
        for d in (self.work_dir, self.download_dir, self.src_dir):
            os.makedirs(d, exist_ok=True)
        exe = os.path.join(
            get_package_prefix("service_ota_app"), "lib", "service_ota_app",
            "app_ota_manager_node")
        cmd = [
            exe,
            "--ros-args",
            "-p", "work_dir:=" + self.work_dir,
            "-p", "download_dir:=" + self.download_dir,
            "-p", "cart_id:=" + cart_id,
        ]
        self.log_path = os.path.join(base_dir, "node.log")
        self._log = open(self.log_path, "w")
        self.proc = subprocess.Popen(
            cmd, stdout=self._log, stderr=subprocess.STDOUT,
            start_new_session=True)
        self.cart_id = cart_id

    def alive(self):
        return self.proc.poll() is None

    def stop(self):
        if self.proc.poll() is None:
            try:
                os.killpg(os.getpgid(self.proc.pid), signal.SIGTERM)
            except ProcessLookupError:
                pass
            try:
                self.proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                os.killpg(os.getpgid(self.proc.pid), signal.SIGKILL)
                self.proc.wait(timeout=5)
        self._log.close()

    def node_log(self):
        try:
            with open(self.log_path, "r") as f:
                return f.read()
        except OSError:
            return ""


failures = []


def check(cond, name, detail=""):
    if cond:
        print("  [PASS] %s" % name)
    else:
        print("  [FAIL] %s %s" % (name, detail))
        failures.append(name)


def make_payload_image(src_dir, name, content):
    path = os.path.join(src_dir, name)
    with open(path, "wb") as f:
        f.write(content)
    return path, hashlib.sha256(content).hexdigest()


def make_command(task_id, module, url, sha256, version="v9.9.9-l2",
                 target_model="", delivery=TAR, soc=0):
    cmd = OtaCommand()
    now_ms = int(time.time() * 1000)
    cmd.header.msg_id = task_id + "-msg"
    cmd.header.timestamp_ms = now_ms
    cmd.header.protocol_version = "1.3"
    cmd.header.cart_id = "cart-l2"
    cmd.ota_task_id = task_id
    cmd.target_module = module
    cmd.target_model = target_model
    cmd.target_version = version
    cmd.url = url
    cmd.sha256 = sha256
    cmd.signature = ""
    cmd.delivery = delivery
    cmd.min_battery_soc = soc
    return cmd


def all_statuses_have_task_id(statuses, task_id):
    return all(s.ota_task_id == task_id for s in statuses)


def run_case_app(harness):
    print("[Case A] APP 正常升级 + 连续第二个任务")
    base = tempfile.mkdtemp(prefix="ota_l2_app_")
    node = OtaNodeProc(base)
    try:
        check(harness.wait_ready(), "节点就绪（/ota/status 有发布、/ota/command 有订阅）")
        harness.reset()
        src, sha = make_payload_image(node.src_dir, "app.tar", b"fake docker image A")
        cmd = make_command("L2-APP-OK", APP, "file://" + src, sha, soc=30)
        harness.pub.publish(cmd)

        ok = harness.wait_until(
            lambda st: any(s.stage == STAGE_INSTALLING and s.progress >= 90 for s in st)
            or any(s.result != RESULT_NONE for s in st), 20.0)
        check(ok, "收到 INSTALLING 中间态", "statuses=%d" % len(harness.statuses))
        check(len(harness.statuses) > 0, "至少收到一条 OtaStatus")
        check(all_statuses_have_task_id(harness.statuses, "L2-APP-OK"),
              "所有进度都携带 ota_task_id", [s.ota_task_id for s in harness.statuses])

        stages = [s.stage for s in harness.statuses]
        check(stages and stages[0] == STAGE_DOWNLOADING, "首帧为 DOWNLOADING", stages[:3])
        check(STAGE_VERIFYING in stages, "出现 VERIFYING")
        check(STAGE_INSTALLING in stages, "出现 INSTALLING")

        last = harness.statuses[-1] if harness.statuses else None
        check(last is not None, "收到状态消息")
        if last is None:
            return
        check(last.result == RESULT_NONE and last.progress >= 90,
              "握手后为等待宿主执行态(progress>=90,result=无)",
              "progress=%d result=%d" % (last.progress, last.result))
        check(last.header.protocol_version == "1.3", "protocol_version=1.3")
        check(last.header.cart_id == "cart-l2", "cart_id 透传", last.header.cart_id)
        check(bool(last.header.msg_id) and last.header.timestamp_ms > 0,
              "msg_id/timestamp_ms 已填充")

        req_path = os.path.join(node.work_dir, "request.json")
        check(os.path.isfile(req_path), "request.json 已写入")
        if os.path.isfile(req_path):
            with open(req_path) as f:
                req = json.load(f)
            check(req.get("ota_task_id") == "L2-APP-OK", "request.ota_task_id")
            check(req.get("target_version") == "v9.9.9-l2", "request.target_version")
            check(req.get("action") == "install", "request.action=install")
            check(req.get("delivery") == "tar", "request.delivery=tar")
            check(req.get("tar_path", "").endswith("app.tar"), "request.tar_path")
            check(req.get("sha256") == sha, "request.sha256")
            check(req.get("min_battery_soc") == 30, "request.min_battery_soc=30")

        # --- 连续第二个任务：验证 worker_ 复用不崩溃 ---
        harness.pump(1.0)
        check(node.alive(), "首个任务结束后节点仍存活")
        harness.reset()
        src2, sha2 = make_payload_image(node.src_dir, "app2.tar", b"fake docker image B")
        harness.pub.publish(make_command("L2-APP-2", APP, "file://" + src2, sha2, soc=50))
        ok2 = harness.wait_until(
            lambda st: any(s.stage == STAGE_INSTALLING and s.progress >= 90 for s in st),
            20.0)
        check(ok2, "第二个任务能推进到 INSTALLING", "statuses=%d" % len(harness.statuses))
        check(node.alive(), "第二个任务期间节点存活（未触发 terminate）")
        check(all_statuses_have_task_id(harness.statuses, "L2-APP-2"),
              "第二个任务进度 task_id 正确")
    finally:
        node.stop()
        if failures:
            print("---- 节点日志（%s）----\n%s" % (node.log_path, node.node_log()[-3000:]))
        shutil.rmtree(base, ignore_errors=True)


def run_case_bad_sha(harness):
    print("[Case B] sha256 不匹配 -> FAILED")
    base = tempfile.mkdtemp(prefix="ota_l2_bad_")
    node = OtaNodeProc(base)
    try:
        check(harness.wait_ready(), "节点就绪")
        harness.reset()
        src, _ = make_payload_image(node.src_dir, "bad.tar", b"corrupted content")
        bad = "0" * 64
        harness.pub.publish(make_command("L2-BAD-SHA", APP, "file://" + src, bad))
        ok = harness.wait_until(
            lambda st: any(s.result == RESULT_FAILED for s in st), 20.0)
        check(ok, "收到 FAILED 终态", "statuses=%d" % len(harness.statuses))
        if harness.statuses:
            last = harness.statuses[-1]
            check(last.result == RESULT_FAILED, "result=FAILED")
            check(last.stage in (STAGE_DOWNLOADING, STAGE_VERIFYING),
                  "失败发生在下载/校验阶段", "stage=%d" % last.stage)
            check(last.ota_task_id == "L2-BAD-SHA", "失败状态携带 task_id")
        check(not os.path.exists(os.path.join(node.work_dir, "request.json")),
              "校验失败不写 request.json（未进入安装）")
        check(not os.path.exists(os.path.join(node.download_dir, "bad.tar")),
              "校验失败文件被清理")
        check(node.alive(), "失败后节点存活")
    finally:
        node.stop()
        if failures:
            print("---- 节点日志（%s）----\n%s" % (node.log_path, node.node_log()[-3000:]))
        shutil.rmtree(base, ignore_errors=True)


def run_case_chassis(harness):
    print("[Case C] CHASSIS 任务字段透传")
    base = tempfile.mkdtemp(prefix="ota_l2_chassis_")
    node = OtaNodeProc(base)
    try:
        check(harness.wait_ready(), "节点就绪")
        harness.reset()
        src, sha = make_payload_image(node.src_dir, "stm32.bin", b"fake stm32 firmware")
        harness.pub.publish(make_command(
            "L2-CHASSIS", CHASSIS, "file://" + src, sha,
            version="v1.1.5", target_model="STM32G474"))
        ok = harness.wait_until(
            lambda st: any(s.stage == STAGE_INSTALLING and s.progress >= 90 for s in st),
            20.0)
        check(ok, "CHASSIS 任务推进到 INSTALLING")
        if harness.statuses:
            last = harness.statuses[-1]
            check(last.target_module == CHASSIS, "OtaStatus.target_module=CHASSIS",
                  "got %d" % last.target_module)
            check(last.target_version == "v1.1.5", "target_version 透传")
        req_path = os.path.join(node.work_dir, "request.json")
        check(os.path.isfile(req_path), "request.json 已写入")
        if os.path.isfile(req_path):
            with open(req_path) as f:
                req = json.load(f)
            check(req.get("delivery") == "tar", "CHASSIS 走 tar 分发")
            check(req.get("tar_path", "").endswith("stm32.bin"), "固件落盘路径")
    finally:
        node.stop()
        if failures:
            print("---- 节点日志（%s）----\n%s" % (node.log_path, node.node_log()[-3000:]))
        shutil.rmtree(base, ignore_errors=True)


def main():
    rclpy.init()
    harness = Harness()
    try:
        run_case_app(harness)
        run_case_bad_sha(harness)
        run_case_chassis(harness)
    finally:
        harness.destroy_node()
        rclpy.shutdown()

    print()
    if failures:
        print("L2 验证失败：%d 项" % len(failures))
        for name in failures:
            print("  - " + name)
        return 1
    print("L2 验证通过：OTA 消息流与契约转换符合预期")
    return 0


if __name__ == "__main__":
    sys.exit(main())
