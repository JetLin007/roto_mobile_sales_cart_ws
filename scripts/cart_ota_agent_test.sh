# 旧用法（环境变量）
OTA_MOCK_MODE=1 OTA_MOCK_RESULT=success OTA_DIR=/tmp/ota_test python3 scripts/cart_ota_agent.py

# 新用法（CLI 参数，优先）
python3 scripts/cart_ota_agent.py -m -r rollback_failed -d /tmp/ota_test
python3 scripts/cart_ota_agent.py --help