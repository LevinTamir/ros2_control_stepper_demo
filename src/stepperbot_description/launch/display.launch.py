from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, Command, PathJoinSubstitution
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    pkg_share = get_package_share_directory("stepperbot_description")

    urdf_file = PathJoinSubstitution(
        [pkg_share, "urdf", "stepperbot.urdf.xacro"]
    )

    rviz_config_file = PathJoinSubstitution(
        [pkg_share, "rviz", "urdf_config.rviz"]
    )

    robot_description = Command(["xacro ", urdf_file])

    return LaunchDescription([
        Node(
            package="robot_state_publisher",
            executable="robot_state_publisher",
            parameters=[{"robot_description": robot_description}],
            output="screen",
        ),
        Node(
            package="rviz2",
            executable="rviz2",
            name="rviz2",
            arguments=["-d", rviz_config_file],
            output="screen",
        ),
    ])
