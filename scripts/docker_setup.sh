# 把容器 stop → commit → 带新 restart policy run
docker commit roto-builder roto-builder-fixed
docker rm roto-builder
docker run -d --name roto-builder \
    --restart=unless-stopped \
    --hostname rk3576-builder \
    --network host \
    -v /data/workspace/roto_mobile_sales_cart_ws:/ws \
    -v /etc/localtime:/etc/localtime:ro \
    roto-builder-fixed sleep infinity


docker exec -it roto-builder bash
source /opt/ros/humble/setup.bash && source /ws/install/setup.bash

pkill -f comm_mqtt_cloud 2>/dev/null || true
pkill -f app_ota_manager 2>/dev/null || true
sleep 1

# bridge 后台
ros2 run service_cloud_comm comm_mqtt_cloud --ros-args \
  -p cloud_broker:=localhost -p cloud_port:=1883 -p cloud_tls:=false -p device_id:=cart-test &

# ota_app 前台（看实时日志）
ros2 run service_ota_app app_ota_manager_node --ros-args \
  --params-file /opt/roto_ws/data/ota/params.yaml