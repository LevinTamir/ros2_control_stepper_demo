cmake_minimum_required(VERSION 3.8)
project(stepperbot_description)

find_package(ament_cmake REQUIRED)

# Install URDF/xacro files
install(
  DIRECTORY urdf
  DESTINATION share/${PROJECT_NAME}
)

# Install launch files
install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

# Install RViz configs
install(
  DIRECTORY rviz
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
