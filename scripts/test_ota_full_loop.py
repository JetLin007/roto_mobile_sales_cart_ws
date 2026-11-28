#!/usr/bin/env python3
# =============================================================================
# test_ota_full_loop.py — OTA 完整闭环自动化测试脚本
# -----------------------------------------------------------------------------
# 前提（由脚本自动检查）：
#   1. ROS2 环境已 source (/opt/ros/humble/setup.bash  + /ws/install/setup.bash)
#   2. MQTT broker 本地 1883 端口可连（mock-mqtt 容器）
#   3. /ws/scripts/cart_ota_agent.py 可执行 + python3 paho.mqtt.client 已装
#   4. 一个可用的 HTTP server 提供 tarball 下载（脚本自己起）
#
# 覆盖场景（用 --mock-result 切换）：
#   success            默认成功路径
#   failed             模拟 agent load/pull 失败（result.json status=failed）
#   rollback_success   模拟健康检查超时→回滚成功
#   rollback_failed    模拟健康检查超时+无 previous→回滚失败
#
# 用法示例：
#   source /opt/ros/humble/setup.bash && source /ws/install/setup.bash
#   python3 scripts/test_ota_full_loop.py
#   python3 scripts/test_ota_full_loop.py --mock-result rollback_success
#   python3 scripts/test_ota_full_loop.py --keep   # 保留测试文件（方便排查）
# =============================================================================
import argparse
import hashlib
import json
import os
import shutil
import socket
import ssl
import stat
import subprocess
import sys
import tempfile
import time
from pathlib import Path

# ---------------------------------------------------------------------------
# 颜色输出（tty 自动开启）
# ---------------------------------------------------------------------------
class T:
    OK   = "\033[92m"
    WARN = "\033[93m"
    FAIL = "\033[91m"
    BOLD = "\033[1m"
    RESET = "\033[0m"

def _tty(): return sys.stdout.isatty()

def ok(s):   print(f"{T.OK if _tty() else ''}  ✅ {s}{T.RESET if _tty() else ''}")
def fail(s): print(f"{T.FAIL if _tty() else ''}  ❌ {s}{T.RESET if _tty() else ''}")
def info(s): print(f"{T.BOLD if _tty() else ''}  ▶ {s}{T.RESET if _tty() else ''}")
def warn(s): print(f"{T.WARN if _tty() else ''}  ⚠  {s}{T.RESET if _tty() else ''}")

# ---------------------------------------------------------------------------
# 工具函数
# ---------------------------------------------------------------------------
def wait_port(host, port, timeout_s=5):
    """轮询 TCP 端口直到可用。"""
    deadline = time.time() + timeout_s
    while time.time() < deadline:
        try:
            with socket.create_connection((host, port), timeout=0.5):
                return True
        except (ConnectionRefusedError, OSError):
            time.sleep(0.3)
    return False

def run(cmd, **kw):
    """运行命令并打印。"""
    info(f"$ {' '.join(cmd)}")
    return subprocess.run(cmd, **kw)

# ---------------------------------------------------------------------------
# OpenSSL Ed25519 签名辅助（调用 openssl 命令行）
# ---------------------------------------------------------------------------
def gen_ed25519_keys(priv_path, pub_path):
    subprocess.run(["openssl", "genpkey", "-algorithm", "Ed25519",
                    "-out", str(priv_path)], check=True, capture_output=True)
    subprocess.run(["openssl", "pkey", "-in", str(priv_path), "-pubout",
                    "-out", str(pub_path)], check=True, capture_output=True)

def ed25519_sign_file(file_path, priv_path):
    """返回 base64 编码的 Ed25519 分离签名。"""
    cp = subprocess.run(
        ["openssl", "pkeyutl", "-sign",
         "-inkey", str(priv_path),
         "-in", str(file_path),
         "-rawin"],
        check=True, capture_output=True,
    )
    cp2 = subprocess.run(["base64", "-w0"], input=cp.stdout,
                          capture_output=True)
    return cp2.stdout.decode().strip()

def sha256_file(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

# ---------------------------------------------------------------------------
# paho-mqtt 辅助（延迟 import，避免脚本开头就因没装而崩）
# ---------------------------------------------------------------------------
def _mqtt():
    try:
        import paho.mqtt.client as mqtt
        return mqtt
    except ImportError:
        fail("缺少 paho.mqtt.client，运行: pip3 install paho-mqtt")
        sys.exit(2)

def mqtt_subscribe(topic, timeout_s=20):
    """订阅并阻塞等待一条消息，返回 payload dict；超时返回 None。"""
    mqtt_mod = _mqtt()
    received = [None]

    def on_msg(c, u, m):
        try:
            received[0] = json.loads(m.payload.decode())
        except Exception as e:
            received[0] = {"_raw": m.payload.decode(), "_err": str(e)}

    client = mqtt_mod.Client()
    client.on_message = on_msg
    client.connect("localhost", 1883)
    client.subscribe(topic, qos=1)
    client.loop_start()

    deadline = time.time() + timeout_s
    while time.time() < deadline and received[0] is None:
        time.sleep(0.1)
    client.loop_stop()
    client.disconnect()
    return received[0]

def mqtt_publish(topic, payload):
    mqtt_mod = _mqtt()
    client = mqtt_mod.Client()
    client.connect("localhost", 1883)
    client.publish(topic, json.dumps(payload), qos=1)
    client.disconnect()

# ---------------------------------------------------------------------------
# 主流程
# ---------------------------------------------------------------------------
def main():
    ap = argparse.ArgumentParser(description="OTA 完整闭环自动化测试")
    ap.add_argument("--mock-result", default="success",
                    choices=("success", "failed", "rollback_success", "rollback_failed"),
                    help="cart_ota_agent mock 模式下写的 result.json status")
    ap.add_argument("--ota-dir", default=None,
                    help="OTA 握手目录（默认自动创建临时目录）")
    ap.add_argument("--keep", action="store_true",
                    help="测试结束后保留临时文件（方便排查）")
    ap.add_argument("--url-host", default="http://127.0.0.1",
                    help="tarball 下载 URL 主机（默认 http://127.0.0.1）")
    ap.add_argument("--url-port", default="18080", type=int,
                    help="tarball HTTP 端口（默认 18080）")
    args = ap.parse_args()

    print()
    print(f"{T.BOLD if _tty() else ''}=== OTA 完整闭环自动化测试 启动 ==={T.RESET if _tty() else ''}")
    print(f"  mock_result = {args.mock_result}")
    print(f"  ota_dir     = {args.ota_dir or '(临时目录)'}")
    print()

    # ---- 1. 环境预检 ----
    info("环境预检")
    errors = []
    if not wait_port("localhost", 1883, timeout_s=3):
        errors.append("MQTT broker localhost:1883 连不上（mock-mqtt 容器在跑吗？）")
    if not shutil.which("ros2"):
        errors.append("ros2 命令不存在（source /opt/ros/humble/setup.bash 了吗？）")
    if not shutil.which("openssl"):
        errors.append("openssl 命令不存在（apt install openssl）")
    if not Path("/ws/scripts/cart_ota_agent.py").is_file():
        errors.append("/ws/scripts/cart_ota_agent.py 不存在")
    try:
        _mqtt()
    except SystemExit:
        return 2
    if errors:
        for e in errors: fail(e)
        return 1
    ok("通过")

    # ---- 2. 创建目录 + 密钥 + tarball ----
    info("准备测试素材")
    tmp_root = tempfile.mkdtemp(prefix="ota_test_")
    ota_dir  = Path(args.ota_dir) if args.ota_dir else Path(tmp_root) / "ota"
    cache_dir = ota_dir / "cache"
    ota_dir.mkdir(parents=True, exist_ok=True)
    cache_dir.mkdir(parents=True, exist_ok=True)

    key_dir   = Path(tmp_root) / "keys"
    tar_dir   = Path(tmp_root) / "tarball"
    key_dir.mkdir(exist_ok=True)
    tar_dir.mkdir(exist_ok=True)

    # 密钥
    priv = key_dir / "ed25519_priv.pem"
    pub  = key_dir / "ed25519_pub.pem"
    gen_ed25519_keys(priv, pub)
    pub_pem = pub.read_text()
    ok("Ed25519 密钥对生成")

    # 伪造 tarball
    (tar_dir / "app.bin").write_bytes(b"fake roto-cart image v2.0.2\n" * 100)
    tar_path = tar_dir / "app_v2.0.2.tar.gz"
    subprocess.run(["tar", "-czf", str(tar_path), "-C", str(tar_dir), "app.bin"],
                   check=True, capture_output=True)
    sha256 = sha256_file(tar_path)
    sig_b64 = ed25519_sign_file(tar_path, priv)
    ok(f"tarball 生成: sha256={sha256[:16]}...  size={tar_path.stat().st_size}B")

    # ---- 3. 起 HTTP server（子进程）----
    info(f"起 HTTP server 端口 {args.url_port}")
    http_port = args.url_port
    tar_url = f"{args.url_host}:{http_port}/app_v2.0.2.tar.gz"
    # 先清上次失败留下的僵尸 http.server，避免端口被旧进程占坑
    subprocess.run(["pkill", "-f", f"http.server {http_port}"],
                   capture_output=True)
    time.sleep(0.5)
    http_proc = subprocess.Popen(
        [sys.executable, "-m", "http.server", str(http_port),
         "--directory", str(tar_dir), "--bind", "127.0.0.1"],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    )
    if not wait_port("127.0.0.1", http_port, timeout_s=3):
        fail("HTTP server 没起来")
        http_proc.kill()
        return 1
    # 关键：主动 curl 验证它 serving 的确实是当前脚本生成的 tarball
    # 而不是某个僵尸进程留下的旧文件
    cp = subprocess.run(
        ["curl", "-fsSL", "-o", str(tar_dir / ".__verify__.bin"),
         tar_url],
        capture_output=True, text=True, timeout=3,
    )
    if cp.returncode != 0:
        fail(f"curl 验证失败: {cp.stderr}")
        http_proc.kill()
        return 1
    verify_sha = sha256_file(tar_dir / ".__verify__.bin")
    if verify_sha != sha256:
        fail(f"HTTP server serving 了错误的文件！本 tarball sha256={sha256[:16]} 但 curl 回来的是 {verify_sha[:16]}")
        fail("大概率是上次失败的脚本 run 留下的僵尸 http.server 占着同一个端口")
        http_proc.kill()
        return 1
    ok(f"HTTP server ready + verified (sha256={sha256[:16]}...)")

    # ---- 4. 起 bridge + ota_app（ros2 run 后台）----
    info("启动 ros2 节点")

    def kill_ros():
        for pat in ("comm_mqtt_cloud", "app_ota_manager"):
            subprocess.run(["pkill", "-f", pat], capture_output=True)

    kill_ros()
    time.sleep(0.5)

    bridge_log = Path(tmp_root) / "bridge.log"
    ota_log    = Path(tmp_root) / "ota.log"
    bridge_proc = subprocess.Popen(
        ["ros2", "run", "service_cloud_comm", "comm_mqtt_cloud",
         "--ros-args",
         "-p", "cloud_broker:=localhost",
         "-p", "cloud_port:=1883",
         "-p", "cloud_tls:=false",
         "-p", "device_id:=cart-test"],
        stdout=bridge_log.open("w"), stderr=subprocess.STDOUT,
    )
    # 写 params.yaml（YAML 字符串传递多行 PEM）
    params_yaml = Path(tmp_root) / "params.yaml"
    params_yaml.write_text(
        "app_ota_manager:\n"         # 必须与 Node("app_ota_manager") 严格一致
        "  ros__parameters:\n"
        f"    work_dir: {ota_dir}\n"
        f"    download_dir: {cache_dir}\n"
        "    cart_id: cart-test\n"
        "    ed25519_pubkey_pem: |\n"
        + "\n".join("      " + line for line in pub_pem.strip().splitlines())
        + "\n"
    )

    ota_proc = subprocess.Popen(
        ["ros2", "run", "service_ota_app", "app_ota_manager_node",
         "--ros-args", "--params-file", str(params_yaml)],
        stdout=ota_log.open("w"), stderr=subprocess.STDOUT,
    )
    # 等节点真正起来（用 ros2 node list 轮询，比 sleep 瞎等可靠）
    deadline = time.time() + 10
    nodes = []
    while time.time() < deadline:
        cp = subprocess.run(["ros2", "node", "list"],
                            capture_output=True, text=True, timeout=2)
        nodes = [n.strip() for n in cp.stdout.splitlines()]
        if any("comm_mqtt_cloud" in n for n in nodes) and \
           any("app_ota_manager" in n for n in nodes):
            break
        time.sleep(0.5)
    ok(f"bridge + ota_app 启动 (nodes: {nodes})")

    # 关键：验证 params.yaml 里的 work_dir 确实被 ota_app 读到了
    # 否则会偷偷走默认路径 /opt/roto_ws/data/ota，脚本找错目录
    param_cp = subprocess.run(
        ["ros2", "param", "get", "/app_ota_manager", "work_dir"],
        capture_output=True, text=True, timeout=3,
    )
    got_work_dir = param_cp.stdout.strip()
    # ros2 param get Humble 版输出: "String value is: /path/to/dir"
    if ":" in got_work_dir:
        got_work_dir = got_work_dir.split(":", 1)[1].strip()
    got_work_dir = got_work_dir.strip('"')
    if got_work_dir != str(ota_dir):
        fail(f"params.yaml 未生效！work_dir = {got_work_dir!r}  期望 {str(ota_dir)!r}")
        fail("检查 params.yaml 顶层 key 是否为 'app_ota_manager'（必须与节点名完全一致）")
        return 1
    ok(f"params.yaml 生效 work_dir={got_work_dir}")

    # ---- 5. 等 MQTT ota/status 初始订阅建立 ----
    # 先发一条垃圾消息看 bridge 有没有转发 -> 跳过，直接发指令
    info("发 MQTT OTA 指令 + 同时订阅最终 status")
    TASK_ID = f"task-auto-{int(time.time())}"
    ota_cmd = {
        "msg_id": "uuid-auto-001",
        "timestamp": int(time.time() * 1000),
        "ota_task_id": TASK_ID,
        "target_module": "APP",
        "target_version": "2.0.2",
        "image_url": tar_url,
        "sha256": sha256,
        "signature": sig_b64,
        "delivery": "tar",
        "min_battery_soc": 10,
    }

    # 先发 MQTT 指令
    mqtt_publish("ota/app/version", ota_cmd)
    ok(f"MQTT 指令已发 task={TASK_ID}")

    # ---- 6. 等 request.json 出现 ----
    info("等待 ota_app 下载 → 校验 → handoff")
    deadline = time.time() + 15
    while time.time() < deadline:
        if (ota_dir / "request.json").is_file():
            break
        time.sleep(0.2)

    req_file = ota_dir / "request.json"
    if not req_file.is_file():
        fail("request.json 没出现 —— dump 日志如下")
        print()
        print(f"--- bridge.log ({bridge_log}) ---")
        if bridge_log.is_file(): print(bridge_log.read_text() or "(空)")
        print(f"--- ota.log ({ota_log}) ---")
        if ota_log.is_file(): print(ota_log.read_text() or "(空)")
        print(f"--- params.yaml ---")
        print(params_yaml.read_text())
        print(f"--- ota_dir 内容 ---")
        for p in ota_dir.rglob("*"): print(f"  {p}")
        print(f"--- tar_url (curl 手动测一下) ---")
        print(f"  curl -s -o /dev/null -w '%{{http_code}}' {tar_url}")
        return 1
    ok(f"request.json 存在 ({req_file.stat().st_size}B)")

    # ---- 7. 跑 cart_ota_agent mock 模式 ----
    info(f"启动 cart_ota_agent mock (result={args.mock_result})")
    agent_proc = subprocess.Popen(
        ["python3", "/ws/scripts/cart_ota_agent.py",
         "-m", "-r", args.mock_result,
         "-d", str(ota_dir)],
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
        env={**os.environ, "OTA_DIR": str(ota_dir),
             "OTA_MOCK_MODE": "1", "OTA_MOCK_RESULT": args.mock_result},
    )
    # 等它处理完（request.json 被删）
    deadline = time.time() + 10
    while time.time() < deadline:
        if not req_file.is_file():
            break
        time.sleep(0.2)
    try:
        agent_out, _ = agent_proc.communicate(timeout=8)
    except subprocess.TimeoutExpired:
        agent_proc.kill()
        agent_out, _ = agent_proc.communicate()

    # agent 日志
    for line in agent_out.splitlines():
        info(f"  agent  {line}")
    ok("agent 处理完成")

    result_file = ota_dir / "result.json"
    if not result_file.is_file():
        fail("result.json 没出现")
        return 1
    result_json = json.loads(result_file.read_text())
    ok(f"result.json: status={result_json.get('status')}  message={result_json.get('message')}")

    # ---- 8. kill ota_app，然后重启它（模拟容器重启 → report_last_result）----
    info("kill ota_app → 重启（模拟容器重启）")
    ota_proc.terminate()
    try:
        ota_proc.wait(timeout=3)
    except subprocess.TimeoutExpired:
        ota_proc.kill()

    # 订阅最终 status（先订阅再重启，保证不丢）
    sub_thread_result = [None]

    import threading
    def _sub():
        sub_thread_result[0] = mqtt_subscribe("ota/status", timeout_s=15)
    sub_thread = threading.Thread(target=_sub, daemon=True)
    sub_thread.start()
    time.sleep(0.3)

    ota_proc2 = subprocess.Popen(
        ["ros2", "run", "service_ota_app", "app_ota_manager_node",
         "--ros-args", "--params-file", str(params_yaml)],
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
    )
    sub_thread.join(timeout=18)
    final_status = sub_thread_result[0]

    # ---- 9. 清理 + 断言 ----
    ok("--- 最终 status 断言 ---")
    all_pass = True

    expected_stage  = {
        "success":          "FINISHED",
        "failed":           "FINISHED",
        "rollback_success": "FINISHED",
        "rollback_failed":  "FINISHED",
    }[args.mock_result]

    expected_result = {
        "success":          "SUCCESS",
        "failed":           "FAILED",
        "rollback_success": "ROLLBACK_SUCCESS",
        "rollback_failed":  "ROLLBACK_FAILED",
    }[args.mock_result]

    if final_status is None:
        fail("MQTT ota/status 没收到最终消息")
        all_pass = False
    else:
        got_stage  = final_status.get("stage")
        got_result = final_status.get("result")
        got_task   = final_status.get("ota_task_id")

        (ok if got_stage == expected_stage else fail)(
            f"stage = {got_stage!r}  (期望 {expected_stage!r})")
        (ok if got_result == expected_result else fail)(
            f"result = {got_result!r}  (期望 {expected_result!r})")
        (ok if got_task == TASK_ID else fail)(
            f"ota_task_id = {got_task!r}  (期望 {TASK_ID!r})")
        all_pass = (got_stage == expected_stage and got_result == expected_result
                    and got_task == TASK_ID)

    # ---- 10. 收尾 ----
    info("清理子进程")
    bridge_proc.terminate(); bridge_proc.wait()
    ota_proc2.terminate()
    try: ota_proc2.wait(timeout=2)
    except subprocess.TimeoutExpired: ota_proc2.kill()
    http_proc.kill()

    if not args.keep:
        shutil.rmtree(tmp_root, ignore_errors=True)
        if not args.ota_dir:
            shutil.rmtree(ota_dir, ignore_errors=True)
    else:
        info(f"保留文件: {tmp_root}  ota_dir={ota_dir}")

    print()
    if all_pass:
        print(f"{T.OK if _tty() else ''}{T.BOLD if _tty() else ''}🎉 OTA 闭环测试通过！{T.RESET if _tty() else ''}")
        print(f"  mock_result = {args.mock_result}")
        print(f"  预期 stage  = {expected_stage}")
        print(f"  预期 result = {expected_result}")
        return 0
    else:
        print(f"{T.FAIL if _tty() else ''}{T.BOLD if _tty() else ''}❌ OTA 闭环测试失败{T.RESET if _tty() else ''}")
        return 1

if __name__ == "__main__":
    sys.exit(main())