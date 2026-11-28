#!/usr/bin/env python3
"""OTA bridge 双向链路验证脚本。

覆盖三种场景：
  1) 上行（ROS → MQTT）: 向 ROS /ota/status 发模拟 OtaStatus，验证 bridge
     编码后 publish 到 MQTT ota/status。
  2) 下行（MQTT → ROS）: 向 MQTT ota/app/version 发合法 JSON，验证 bridge
     解码后 publish 到 ROS /ota/command。
  3) 拒绝路径: 向 MQTT 发缺字段的非法 JSON，验证 bridge 丢弃不误发 ROS。

运行前提:
  - roto-builder 容器内已 source install/setup.bash
  - comm_mqtt_cloud 正在运行（连接 localhost:1883）
  - MQTT broker 可达
"""
import json
import subprocess
import sys
import time
import threading

import paho.mqtt.client as mqtt


BROKER = "localhost"
PORT = 1883
ROS_ENV = ". /opt/ros/humble/setup.bash && . /ws/install/setup.bash && "
ROS_EXEC = "/bin/bash -c '" + ROS_ENV + "{cmd}'"


class TestResult:
    def __init__(self, name):
        self.name = name
        self.passed = False
        self.detail = ""
        self.expected = ""
        self.actual = ""

    def ok(self, detail=""):
        self.passed = True
        self.detail = detail

    def fail(self, detail, expected="", actual=""):
        self.passed = False
        self.detail = detail
        self.expected = expected
        self.actual = actual

    def __repr__(self):
        mark = "✅ PASS" if self.passed else "❌ FAIL"
        return f"[{mark}] {self.name}\n  detail: {self.detail}"


def run(cmd, timeout=10):
    """Run a shell command inside roto-builder."""
    print(f"  $ {cmd}")
    try:
        result = subprocess.run(
            cmd, shell=True, capture_output=True, text=True, timeout=timeout
        )
        out = result.stdout.strip() + result.stderr.strip()
        if out:
            print(f"  -> {out[:300]}")
        return result.returncode, out
    except subprocess.TimeoutExpired as e:
        return 124, str(e)


# =============================================================================
# 场景 1: 上行 (ROS /ota/status → bridge → MQTT ota/status JSON)
# =============================================================================
def test_uplink_encode():
    r = TestResult("上行编码: OtaStatus → ota/status JSON")
    print(f"\n{r.name}")

    received = []

    def on_msg(client, userdata, msg):
        received.append({"topic": msg.topic, "payload": msg.payload.decode()})

    mqtt_client = mqtt.Client()
    mqtt_client.on_message = on_msg
    mqtt_client.connect(BROKER, PORT)
    mqtt_client.subscribe("ota/status", qos=1)
    mqtt_client.loop_start()
    time.sleep(0.5)

    # 向 ROS /ota/status 发模拟 OtaStatus
    ros_cmd = (
        "ros2 topic pub --once /ota/status robot_msgs/msg/OtaStatus '"
        '{header: {msg_id: "test-up-001", timestamp_ms: 1732700000000, '
        'protocol_version: "1.3", cart_id: "cart-0000001"}, '
        'ota_task_id: "task-20261128-001", '
        'target_module: 0, target_version: "2.0.1", '
        'stage: 0, progress: 42, result: 0}'
        "'"
    )
    cmd = ROS_EXEC.format(cmd=ros_cmd)
    rc, out = run(cmd)
    time.sleep(2.0)
    mqtt_client.loop_stop()
    mqtt_client.disconnect()

    if not received:
        r.fail("未收到 MQTT ota/status 消息",
               expected="bridge 应 publish 一条 ota/status JSON",
               actual="无")
        return r

    try:
        payload = json.loads(received[-1]["payload"])
    except json.JSONDecodeError as e:
        r.fail(f"收到的不是有效 JSON: {e}", actual=received[-1]["payload"])
        return r

    # 字段校验
    checks = {
        "ota_task_id": ("task-20261128-001", payload.get("ota_task_id")),
        "target_module": ("APP", payload.get("target_module")),
        "stage": ("DOWNLOADING", payload.get("stage")),
        "progress": (42, payload.get("progress")),
        "timestamp": (1732700000000, payload.get("timestamp")),
    }
    mismatches = [
        f"{k}: expected={expected}, got={got}"
        for k, (expected, got) in checks.items()
        if expected != got
    ]

    # result 是 RESULT_NONE(0)，协议要求输出 null
    result_ok = payload.get("result") is None

    if mismatches:
        r.fail("字段不匹配", detail="; ".join(mismatches), actual=json.dumps(payload))
    elif not result_ok:
        r.fail("result 非终态应输出 null", actual=f"result={payload.get('result')}")
    else:
        r.ok(f"收到正确 JSON: {json.dumps(payload)}")
    return r


# =============================================================================
# 场景 2: 下行 (MQTT ota/app/version JSON → bridge → ROS /ota/command OtaCommand)
# =============================================================================
def test_downlink_decode():
    r = TestResult("下行解码: ota/app/version JSON → OtaCommand")
    print(f"\n{r.name}")

    # 用 ros2 topic echo 监听
    echo_proc = subprocess.Popen(
        ROS_EXEC.format(cmd="ros2 topic echo /ota/command --once"),
        shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
    )
    time.sleep(1.0)

    # 用 paho-mqtt 往 MQTT 发 JSON
    cmd_json = {
        "msg_id": "uuid-down-001",
        "timestamp": 1732700000000,
        "ota_task_id": "task-20261128-001",
        "target_module": "APP",
        "target_version": "2.0.1",
        "image_url": "https://example.com/app.tar.gz",
        "sha256": "abcd" * 16,
        "signature": "sig_placeholder_ed25519",
        "delivery": "tar",
        "min_battery_soc": 20,
    }
    payload = json.dumps(cmd_json)

    mqtt_client = mqtt.Client()
    mqtt_client.connect(BROKER, PORT)
    mqtt_client.publish("ota/app/version", payload, qos=1)
    mqtt_client.disconnect()

    # 等待 echo_proc 输出
    try:
        stdout, stderr = echo_proc.communicate(timeout=10)
    except subprocess.TimeoutExpired:
        echo_proc.kill()
        stdout, stderr = echo_proc.communicate()
        r.fail("超时未收到 ROS /ota/command",
               expected="OtaCommand 消息被 echo 出来", actual="超时")
        return r

    output = stdout.strip()
    if not output:
        r.fail("ros2 topic echo 无输出", actual="stdout+stderr=" + (stdout + stderr).strip()[:300])
        return r

    # 简单关键字校验（ros2 topic echo 输出是 YAML 格式）
    required_fields = [
        "ota_task_id:",
        "target_module: 0",       # TARGET_APP
        "target_version:",
        "url: https://example.com/app.tar.gz",
        "delivery: 0",            # DELIVERY_TAR
        "min_battery_soc: 20",
    ]
    missing = [f for f in required_fields if f not in output]

    if missing:
        r.fail("字段缺失", detail="缺少: " + ", ".join(missing), actual=output[:500])
    else:
        r.ok(f"OtaCommand 正确解码并发布到 ROS /ota/command")
    return r


# =============================================================================
# 场景 3: 拒绝路径 (非法 JSON → 不发布 ROS)
# =============================================================================
def test_reject_bad_json():
    r = TestResult("错误拒绝: 缺字段 JSON → 丢弃不发 ROS")
    print(f"\n{r.name}")

    # 先清空订阅（上一轮可能残留），然后确认 echo 在超时后退出
    echo_proc = subprocess.Popen(
        ROS_EXEC.format(cmd="timeout 5 ros2 topic echo /ota/command --once"),
        shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
    )
    time.sleep(0.5)

    # 故意缺 timestamp 和 target_module
    bad_json = json.dumps({
        "msg_id": "uuid-bad-001",
        "ota_task_id": "task-reject",
        "target_version": "3.0.0",
    })

    mqtt_client = mqtt.Client()
    mqtt_client.connect(BROKER, PORT)
    mqtt_client.publish("ota/app/version", bad_json, qos=1)
    mqtt_client.disconnect()

    try:
        stdout, stderr = echo_proc.communicate(timeout=10)
    except subprocess.TimeoutExpired:
        echo_proc.kill()
        stdout, stderr = echo_proc.communicate()

    # 验证 echo 在超时后退出且 stdout 无有效消息
    output = stdout.strip()
    # ros2 topic echo 超时退出时 rc = 124 或无内容
    if not output:
        r.ok("非法 JSON 被正确丢弃, ROS /ota/command 无新消息")
    else:
        r.fail("非法 JSON 却发了 ROS 消息!", actual=output[:300])
    return r


# =============================================================================
# main
# =============================================================================
def main():
    print("=" * 60)
    print("OTA Bridge 双向链路验证")
    print("=" * 60)

    tests = [
        ("上行编码", test_uplink_encode),
        ("下行解码", test_downlink_decode),
        ("错误拒绝", test_reject_bad_json),
    ]

    results = []
    for name, fn in tests:
        try:
            results.append(fn())
        except Exception as e:
            tr = TestResult(name)
            tr.fail(f"异常: {e}")
            results.append(tr)

    print("\n" + "=" * 60)
    print("结果汇总")
    print("=" * 60)
    for r in results:
        print(r)

    passed = sum(1 for r in results if r.passed)
    total = len(results)
    print(f"\n{passed}/{total} 通过")
    sys.exit(0 if passed == total else 1)


if __name__ == "__main__":
    main()
