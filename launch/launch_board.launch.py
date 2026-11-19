"""板端真实硬件启动文件（RK3576S）。

不含 sim 工具；HAL 层使用 *_hw 可执行文件连接真实硬件。
容器内通过 `ros2 launch /opt/roto_ws/launch/launch_board.launch.py` 启动。
"""
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        # ---- HAL 层（真实硬件，hw 版本）----
        Node(package='driver_gps', executable='driver_gps_hw', name='driver_gps'),
        Node(package='driver_ultrasonic', executable='driver_ultrasonic_hw', name='driver_ultrasonic'),
        Node(package='driver_gpio', executable='driver_gpio_hw', name='driver_gpio'),
        Node(package='driver_camera', executable='driver_camera_hw', name='driver_camera'),

        # ---- Service 层 ----
        Node(package='service_cloud_comm', executable='comm_mqtt_cloud', name='service_cloud_comm'),
        Node(package='service_local_comm', executable='comm_mqtt_private', name='service_local_comm'),

        # ---- App 层 ----
        Node(package='app_order_task', executable='app_order_task_node', name='app_order_task'),
        Node(package='app_alarm_warn', executable='app_alarm_warn_node', name='app_alarm_warn'),
        Node(package='service_ota_app', executable='app_ota_manager_node', name='service_ota_app'),
        Node(package='app_remote_video', executable='app_remote_video_node', name='app_remote_video'),
    ])
