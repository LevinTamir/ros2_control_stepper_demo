# StepperBot – ROS 2 Control Stepper Motor Demo

StepperBot is a minimal, end-to-end demonstration of controlling a **real stepper motor** using **ros2_control**, an **Arduino Nano**, and a standard **STEP/DIR driver**.

This repository includes:

- custom ros2_control hardware plugin (`stepperbot_controller`)
- URDF + ros2_control description (`stepperbot_description`)
- Arduino firmware project (`stepperbot_firmware`)
- bringup launch to load the robot model, hardware interface, and controllers

Everything is designed to be simple, readable, and extendable.

---

#### Requirements

- **ROS 2 Jazzy** (tested)
- **PlatformIO** VS Code extension (for firmware upload)
- **Arduino Nano**
- **Stepper driver** (A4988, DRV8825, TMC-series, etc.)
- **Stepper motor** (e.g., NEMA17)

---

#### 1. Clone the Workspace

```bash
git clone git@github.com:LevinTamir/ros2_control_stepper_demo.git stepperbot_ws
cd stepperbot_ws
```


#### 2. Flash the Arduino Firmware

The firmware for driving the stepper motor is located in:

```text
stepperbot_ws/
├── src/
└── stepperbot_firmware/ #  <-- HERE
```

To flash the Arduino Nano, install the **PlatformIO IDE extension for VS Code**:

[PlatformIO VS Code Extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)

After installing the extension:

1. Open VS Code  
2. Go to **File → Open Folder** and select `stepperbot_firmware/`  
3. In the PlatformIO toolbar, click **Build** (checkmark icon in bottom bar)  
4. Click **Upload** (right-arrow icon) to flash the firmware

And that's it.

#### 3. Build the ROS 2 Packages

```bash
cd ~/<path-to-dir-location>/stepperbot_ws # Replace with local directory location
colcon build
source install/setup.bash
```

Workspace structure:

```text
stepperbot_ws/
├── src/
│   ├── stepperbot_controller/
│   └── stepperbot_description/
├── install/
├── log/
├── build/   
└── stepperbot_firmware/
```

#### 4. Bringup

After connecting the Arduino and Motor, run:
 ```bash
cd ~/<path-to-dir-location>/stepperbot_ws # Replace with local directory location
ros2 launch stepperbot_controller stepperbot_controllers.launch.py
 ```
If your Arduino Nano is on another port:

```bash
ros2 launch stepperbot_controller stepperbot_controllers.launch.py serial_port:=/dev/<USB-PORT>
```
To launch Rviz, run:

```bash
source install/setup.bash
ros2 launch stepperbot_description display.launch.py
```


To send commands to the motor, run:

```bash
source install/setup.bash
ros2 topic pub /stepper_position_controller/commands \
std_msgs/msg/Float64MultiArray "data: [1.0]" # Change radians as wanted
```


