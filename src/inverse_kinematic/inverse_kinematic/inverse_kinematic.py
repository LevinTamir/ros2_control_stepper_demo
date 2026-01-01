import rclpy
from rclpy.node import Node
from trajectory_msgs.msg import JointTrajectory, JointTrajectoryPoint
import math

class JointTrajectoryPublisher(Node):
    def __init__(self):
        super().__init__('initial_pose_publisher')

        # Create a publisher for 
        self.publisher = self.create_publisher(JointTrajectory, '/arm_controller/joint_trajectory', 10)
        self.trajectory_command = JointTrajectory()
        joint_names = ['joint_1', 'joint_2']
        self.trajectory_command.joint_names = joint_names      
        point = JointTrajectoryPoint()
        joint_angles = self.inverse_kinematics([0.4, 0.2])
        point.positions = joint_angles
        point.velocities = [0.0, 0.0]
        point.time_from_start.sec = 2

        self.trajectory_command.points = [point]

        # Publish the message
        self.get_logger().info('Publishing joint angles...')
        
        
    def send_joint_angles(self):

        while rclpy.ok():
            self.publisher.publish(self.trajectory_command)
            rclpy.spin_once(self, timeout_sec=0.1)

    def inverse_kinematics(x, y, l1=0.25, l2=0.25, d=0.25):
        """
        Inverse kinematics for a planar 2-D delta arm.

        Parameters
        ----------
        x, y : float
            End-effector position
        l1 : float
            Upper arm length
        l2 : float
            Forearm length
        d : float
            Distance between base joints
        elbow : str
            "down" or "up" (assembly mode)

        Returns
        -------
        theta1, theta2 : float
            Base joint angles in radians
        """


        def theta_calc(x, y, l1, l2, d, side ):
            if side == "left":
                k_1 = x + d / 2
            else:
                k_1 = x - d / 2
            k_2 = y
            k_3 = (k_1**2 + k_2**2 + l1**2 - l2**2) / (2 * l1)
            if side == "left":
                theta = math.atan2(k_2, k_1) + math.acos(k_3 / math.sqrt(k_1**2 + k_2**2))
            else:
                theta = math.atan2(k_2, k_1) - math.acos(k_3 / math.sqrt(k_1**2 + k_2**2))
            return theta

        theta1 = theta_calc(x, y, l1, l2, d, "left")
        theta2 = theta_calc(x, y, l1, l2, d, "right")

        return theta1, theta2
    

    #test


