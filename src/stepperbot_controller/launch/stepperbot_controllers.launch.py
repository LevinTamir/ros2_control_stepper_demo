from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, Command, PathJoinSubstitution
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    # Allow overriding the serial port from the command line
    serial_port_arg = DeclareLaunchArgument(
        "serial_port",
        default_value="/dev/ttyUSB0",
        description="Serial port where the Arduino Nano is connected",
    )

    # Package share directories
    description_share = get_package_share_directory("stepperbot_description")
    controller_share = get_package_share_directory("stepperbot_controller")

    # URDF (xacro) path
    urdf_xacro = PathJoinSubstitution(
        [description_share, "urdf", "stepperbot.urdf.xacro"]
    )

    # Controllers YAML (controller manager + controllers)
    controllers_yaml = PathJoinSubstitution(
        [controller_share, "config", "ros2_controllers.yaml"]
    )

    # Build robot_description by running xacro, passing serial_port into the xacro args
    robot_description = Command([
        "xacro ",
        urdf_xacro,
        " serial_port:=", LaunchConfiguration("serial_port"),
    ])

    # Robot State Publisher (publishes TF from robot_description)
    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[{"robot_description": robot_description}],
        output="screen",
    )

    # ros2_control controller manager node
    controller_manager = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[
            {"robot_description": robot_description},
            controllers_yaml,
        ],
        output="screen",
    )

    # Spawners for controllers
    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster"],
        output="screen",
    )

    stepper_position_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["stepper_position_controller"],
        output="screen",
    )

    return LaunchDescription([
        serial_port_arg,
        robot_state_publisher,
        controller_manager,
        joint_state_broadcaster_spawner,
        stepper_position_controller_spawner,
    ])
