source /opt/ros/humble/setup.bash && source /ws/install/setup.bash

# 测试 success 路径（默认）
python3 /ws/scripts/test_ota_full_loop.py

# 测试 rollback_success 路径
python3 /ws/scripts/test_ota_full_loop.py --mock-result rollback_success

# 测试 failed 路径
python3 /ws/scripts/test_ota_full_loop.py --mock-result failed

# 保留中间文件方便排查
python3 /ws/scripts/test_ota_full_loop.py --keep

# 自定义 OTA 目录（多轮测试复用同一目录）
python3 /ws/scripts/test_ota_full_loop.py --ota-dir /tmp/ota_test_auto