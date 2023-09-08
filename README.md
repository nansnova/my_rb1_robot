# my_rb1_robot

Use one of the following command to send velocities and move the robot
- `rosrun teleop_twist_keyboard teleop_twist_keyboard.py cmd_vel:=my_rb1_robot/cmd_vel`
- `rostopic pub /cmd_vel geometry_msgs/Twist "linear:
  x: 0.2
  y: 0.0
  z: 0.0
angular:
  x: 0.0
  y: 0.0
  z: 0.0"`
