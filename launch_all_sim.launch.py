from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        # Driver layer (stub)
        Node(package='driver_gps', executable='driver_gps_stub', name='driver_gps'),
        Node(package='driver_bms', executable='driver_bms_stub', name='driver_bms'),
        Node(package='driver_ultrasonic', executable='driver_ultrasonic_stub', name='driver_ultrasonic'),
        Node(package='driver_can_chassis', executable='driver_can_chassis_stub', name='driver_can_chassis'),
        Node(package='driver_gpio', executable='driver_gpio_stub', name='driver_gpio'),
        Node(package='driver_camera', executable='driver_camera_stub', name='driver_camera'),

        # Sim tools
        Node(package='sim_chassis', executable='sim_chassis', name='sim_chassis'),
        Node(package='sim_tablet', executable='sim_tablet', name='sim_tablet'),

        # App layer
        Node(package='app_order_task', executable='app_order_task_node', name='app_order_task'),
        Node(package='app_status_report', executable='app_status_report_node', name='app_status_report'),
        Node(package='app_alarm_warn', executable='app_alarm_warn_node', name='app_alarm_warn'),
        Node(package='app_ota_manager', executable='app_ota_manager_node', name='app_ota_manager'),
        Node(package='app_remote_video', executable='app_remote_video_node', name='app_remote_video'),
    ])