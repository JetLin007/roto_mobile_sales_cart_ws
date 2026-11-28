"""板端真实硬件启动文件（RK3576S）。

不含 sim 工具；HAL 层使用 *_hw 可执行文件连接真实硬件。
容器内通过 `ros2 launch /opt/roto_ws/launch/launch_board.launch.py` 启动。

OTA 重启兼容:
  - params.yaml 从宿主共享目录 /opt/roto_ws/data/ota/params.yaml 读取
  - 容器重启后 bridge + ota_app 自动带正确参数起来
  - ota_app 启动瞬间 report_last_result() → 上报 OTA 最终状态
"""
import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration
from launch.actions import DeclareLaunchArgument

# OTA 共享目录 (宿主挂载 -v /opt/roto_ws:/opt/roto_ws)
OTA_DIR   = "/opt/roto_ws/data/ota"
PARAMS    = os.path.join(OTA_DIR, "params.yaml")


def generate_launch_description():
    # 如果共享目录里有 params.yaml, 就用它
    ota_params = [PARAMS] if os.path.isfile(PARAMS) else []

    # bridge 默认连本地 mock-mqtt (1883)
    bridge_default = {
        'cloud_broker':  'localhost',
        'cloud_port':    1883,
        'cloud_tls':     False,
        'device_id':     'cart-test',
    }

    return LaunchDescription([
        # ---- HAL 层（真实硬件，hw 版本）----
        Node(package='driver_gps', executable='driver_gps_hw', name='driver_gps'),
        Node(package='driver_ultrasonic', executable='driver_ultrasonic_hw', name='driver_ultrasonic'),
        Node(package='driver_gpio', executable='driver_gpio_hw', name='driver_gpio'),
        Node(package='driver_camera', executable='driver_camera_hw', name='driver_camera'),

        # ---- Service 层 ----
        Node(package='service_cloud_comm', executable='comm_mqtt_cloud',
             name='service_cloud_comm',
             parameters=[bridge_default]),
        Node(package='service_local_comm', executable='comm_mqtt_private', name='service_local_comm'),

        # ---- App 层 ----
        Node(package='app_order_task', executable='app_order_task_node', name='app_order_task'),
        Node(package='app_alarm_warn', executable='app_alarm_warn_node', name='app_alarm_warn'),

        # ota_app: 统一节点名为 app_ota_manager (之前 launch 叫 service_ota_app,
        #          手动跑叫 app_ota_manager, 导致验证脚本 grep 不准)
        Node(package='service_ota_app', executable='app_ota_manager_node',
             name='app_ota_manager',
             parameters=ota_params),

        Node(package='app_remote_video', executable='app_remote_video_node', name='app_remote_video'),
    ])
