#include "stepperbot_controller/stepperbot_hardware.hpp"

#include <cmath>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>

#include "pluginlib/class_list_macros.hpp"

namespace stepperbot_controller
{

namespace
{
speed_t baud_to_speed_t(int baud)
{
  switch (baud)
  {
    case 9600:    return B9600;
    case 19200:   return B19200;
    case 38400:   return B38400;
    case 57600:   return B57600;
    case 115200:  return B115200;
    default:      return B115200;
  }
}
}  // namespace

hardware_interface::CallbackReturn StepperBotHardware::on_init(
  const hardware_interface::HardwareComponentInterfaceParams & params)
{
  if (hardware_interface::SystemInterface::on_init(params) !=
      hardware_interface::CallbackReturn::SUCCESS)
  {
    return hardware_interface::CallbackReturn::ERROR;
  }

  // Parameters from ros2_control <hardware> tag
  port_ = info_.hardware_parameters.at("serial_port");   // e.g. /dev/ttyUSB0
  baud_ = std::stoi(info_.hardware_parameters.at("baud_rate"));
  steps_per_rev_ = std::stod(info_.hardware_parameters.at("steps_per_rev"));

  std::cout << "[StepperBotHardware] Initialized with steps_per_rev = " << steps_per_rev_ << std::endl;

  hw_position_1_ = 0.0;
  hw_velocity_1_ = 0.0;
  hw_position_command_1_ = 0.0;

  hw_position_2_ = 0.0;
  hw_velocity_2_ = 0.0;
  hw_position_command_2_ = 0.0;

  return hardware_interface::CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface>
StepperBotHardware::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;

  // Joint 1
  state_interfaces.emplace_back(
    hardware_interface::StateInterface(
      "stepper_joint_1", hardware_interface::HW_IF_POSITION, &hw_position_1_));
  state_interfaces.emplace_back(
    hardware_interface::StateInterface(
      "stepper_joint_1", hardware_interface::HW_IF_VELOCITY, &hw_velocity_1_));

  // Joint 2
  state_interfaces.emplace_back(
    hardware_interface::StateInterface(
      "stepper_joint_2", hardware_interface::HW_IF_POSITION, &hw_position_2_));
  state_interfaces.emplace_back(
    hardware_interface::StateInterface(
      "stepper_joint_2", hardware_interface::HW_IF_VELOCITY, &hw_velocity_2_));

  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface>
StepperBotHardware::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;

  command_interfaces.emplace_back(
    hardware_interface::CommandInterface(
      "stepper_joint_1", hardware_interface::HW_IF_POSITION, &hw_position_command_1_));

  command_interfaces.emplace_back(
    hardware_interface::CommandInterface(
      "stepper_joint_2", hardware_interface::HW_IF_POSITION, &hw_position_command_2_));

  return command_interfaces;
}

hardware_interface::CallbackReturn
StepperBotHardware::on_configure(const rclcpp_lifecycle::State &)
{
  // Open serial port
  fd_ = ::open(port_.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd_ < 0)
  {
    std::cerr << "[StepperBotHardware] Failed to open " << port_
              << ": " << std::strerror(errno) << std::endl;
    return hardware_interface::CallbackReturn::ERROR;
  }

  if (!configure_port_())
  {
    std::cerr << "[StepperBotHardware] Failed to configure " << port_ << std::endl;
    ::close(fd_);
    fd_ = -1;
    return hardware_interface::CallbackReturn::ERROR;
  }

  ::usleep(200000);  // small delay for Arduino reset

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn
StepperBotHardware::on_activate(const rclcpp_lifecycle::State &)
{
  hw_position_1_ = 0.0;
  hw_velocity_1_ = 0.0;
  hw_position_command_1_ = 0.0;

  hw_position_2_ = 0.0;
  hw_velocity_2_ = 0.0;
  hw_position_command_2_ = 0.0;

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn
StepperBotHardware::on_deactivate(const rclcpp_lifecycle::State &)
{
  if (fd_ >= 0)
  {
    ::close(fd_);
    fd_ = -1;
  }
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type
StepperBotHardware::read(const rclcpp::Time &, const rclcpp::Duration & period)
{
  // Open-loop approximation: position = last command
  // You can replace this with real feedback later.
  hw_position_1_ = hw_position_command_1_;
  hw_velocity_1_ = 0.0;

  hw_position_2_ = hw_position_command_2_;
  hw_velocity_2_ = 0.0;

  (void)period;
  return hardware_interface::return_type::OK;
}

hardware_interface::return_type
StepperBotHardware::write(const rclcpp::Time &, const rclcpp::Duration &)
{
  if (!send_position_command_rad_(hw_position_command_1_, hw_position_command_2_))
  {
    return hardware_interface::return_type::ERROR;
  }
  return hardware_interface::return_type::OK;
}

bool StepperBotHardware::configure_port_()
{
  struct termios tio;
  if (::tcgetattr(fd_, &tio) != 0)
  {
    std::cerr << "[StepperBotHardware] tcgetattr failed: " << std::strerror(errno) << std::endl;
    return false;
  }

  ::cfmakeraw(&tio);
  speed_t speed = baud_to_speed_t(baud_);
  if (::cfsetispeed(&tio, speed) != 0 || ::cfsetospeed(&tio, speed) != 0)
  {
    std::cerr << "[StepperBotHardware] cfsetispeed/cfsetospeed failed: "
              << std::strerror(errno) << std::endl;
    return false;
  }

  tio.c_cflag |= (CLOCAL | CREAD);
  tio.c_cflag &= ~PARENB;
  tio.c_cflag &= ~CSTOPB;
  tio.c_cflag &= ~CSIZE;
  tio.c_cflag |= CS8;
  tio.c_cflag &= ~CRTSCTS;

  tio.c_lflag = 0;
  tio.c_iflag = 0;
  tio.c_oflag = 0;

  tio.c_cc[VMIN]  = 0;
  tio.c_cc[VTIME] = 0;

  if (::tcsetattr(fd_, TCSANOW, &tio) != 0)
  {
    std::cerr << "[StepperBotHardware] tcsetattr failed: " << std::strerror(errno) << std::endl;
    return false;
  }

  return true;
}

bool StepperBotHardware::send_position_command_rad_(double position_rad_1, double position_rad_2)
{
  if (fd_ < 0)
  {
    std::cerr << "[StepperBotHardware] Serial not open" << std::endl;
    return false;
  }

  // rad -> steps for both motors
  double steps_double_1 = position_rad_1 * (steps_per_rev_ / (2.0 * M_PI));
  long steps_1 = std::lround(steps_double_1);

  double steps_double_2 = position_rad_2 * (steps_per_rev_ / (2.0 * M_PI));
  long steps_2 = std::lround(steps_double_2);

  // Send command: "P1 steps_1 P2 steps_2\n"
  std::string line = "P1 " + std::to_string(steps_1) + " P2 " + std::to_string(steps_2) + "\n";

  const char * data = line.c_str();
  ssize_t len = static_cast<ssize_t>(line.size());
  ssize_t written = 0;

  while (written < len)
  {
    ssize_t n = ::write(fd_, data + written, len - written);
    if (n < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }
      std::cerr << "[StepperBotHardware] write failed: " << std::strerror(errno) << std::endl;
      return false;
    }
    written += n;
  }

  return true;
}

}  // namespace stepperbot_controller

PLUGINLIB_EXPORT_CLASS(
  stepperbot_controller::StepperBotHardware,
  hardware_interface::SystemInterface)
