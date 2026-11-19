"""本机仿真启动文件（WSL2 / amd64 开发机）。

包含 sim_chassis / sim_tablet 仿真工具，用于无硬件时的联调。
板端镜像不使用此文件，板端请用 launch/launch_board.launch.py。

sim 包位于 tools/sim/，本机构建需显式包含：
    colcon build --paths src/* tools/sim/sim_chassis tools/sim/sim_tablet
"""
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        # HAL layer (stub)
        Node(package='driver_gps', executable='driver_gps_stub', name='driver_gps'),
        Node(package='driver_ultrasonic', executable='driver_ultrasonic_stub', name='driver_ultrasonic'),
        Node(package='driver_gpio', executable='driver_gpio_stub', name='driver_gpio'),
        Node(package='driver_camera', executable='driver_camera_stub', name='driver_camera'),

        # Sim tools
        Node(package='sim_chassis', executable='sim_chassis', name='sim_chassis'),
        Node(package='sim_tablet', executable='sim_tablet', name='sim_tablet'),

        # App layer
        Node(package='app_order_task', executable='app_order_task_node', name='app_order_task'),
        Node(package='app_alarm_warn', executable='app_alarm_warn_node', name='app_alarm_warn'),
        Node(package='service_ota_app', executable='app_ota_manager_node', name='service_ota_app'),
        Node(package='app_remote_video', executable='app_remote_video_node', name='app_remote_video'),
        # Service layer
        Node(package='service_cloud_comm', executable='comm_mqtt_cloud', name='service_cloud_comm'),
    ])