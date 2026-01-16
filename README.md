# ROS 2 Control Stepper Motor Demo

StepperBot is a minimal, end-to-end demonstration of controlling **real stepper motors** using **ros2_control**, an **Arduino**, and standard **STEP/DIR stepper motor drivers**.

This repository includes:

- bringup launch to load the robot model, hardware interface, and controllers (`stepperbot_bringup`)
- custom ros2_control hardware plugin (`stepperbot_controller`)
- URDF description (`stepperbot_description`)
- Arduino firmware source code (`stepperbot_firmware`)

---

#### Requirements

- **ROS 2 Jazzy** (tested).
- **PlatformIO** - via VS Code extension.
- **Arduino Uno** - with a CNC sheild.
- **Stepper motor drivers** - two A4988 style motor drivers.
- **Stepper motors** - two Nema 17 stepper motors.

---

#### 1. Clone the Workspace

```bash
git clone git@github.com:LevinTamir/ros2_control_stepper_demo.git stepperbot_ws
cd stepperbot_ws/
```


#### 2. Flash the Arduino Firmware

The firmware for driving the stepper motor is located in:

```text
stepperbot_ws/
├── src/
└── stepperbot_firmware/ #  <-- HERE
```

To flash the Arduino Nano, install the [PlatformIO VS Code Extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide).

After installing the extension:

1. Open VS Code  
2. Go to **File → Open Folder** and select `stepperbot_firmware/`  
3. In the PlatformIO toolbar, click **Build** (checkmark icon in the bottom bar)  
4. Click **Upload** (right-arrow icon) to flash the firmware

And that's it.

#### 3. Build the ROS 2 Packages

```bash
cd ~/<path-to-dir-location>/stepperbot_ws # Replace with your local directory location
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
source install/setup.bash
ros2 launch stepperbot_controller stepperbot_controllers.launch.py
 ```
If your Arduino Nano is on another port:

```bash
source install/setup.bash
ros2 launch stepperbot_controller stepperbot_controllers.launch.py serial_port:=/dev/<USB-PORT>
```
To launch Rviz, run:

```bash
source install/setup.bash
ros2 launch stepperbot_description display.launch.py
```


To send commands to the motor, run:

```bash
ros2 topic pub /stepper_position_controller/commands \
std_msgs/msg/Float64MultiArray "data: [1.0]" # Change radians as wanted
```


