#include <cmath>
#include <geometry_msgs/Twist.h>
#include <my_rb1_ros/Rotate.h> //custom service message
#include <ros/ros.h>
#include <string>

ros::Publisher vel_pub;
geometry_msgs::Twist command;

// global
bool rotation_in_progress = false;

void rotate(double angular_velocity) {
  command.angular.z = angular_velocity;
  vel_pub.publish(command);
}

bool rotateRobotCallback(my_rb1_ros::Rotate::Request &req,
                         my_rb1_ros::Rotate::Response &res) {
  if (rotation_in_progress) {
    res.success = "Rotation is already in progress.";
    return false;
  }

  double target_angle_degrees = req.degrees;
  double target_angle_radians = target_angle_degrees * M_PI / 180.0;

  ROS_INFO("Rotation requested: %.2f degrees", target_angle_degrees);

  // Set the maximum rotation velocity
  double max_angular_velocity = 0.5; // rad/sec

  // Calculate the required time for the rotation
  double rotation_time = fabs(target_angle_radians / max_angular_velocity);

  // Set the rotation direction based on the sign of target_angle_degrees
  double angular_velocity =
      (target_angle_degrees > 0) ? max_angular_velocity : -max_angular_velocity;

  // Start the rotation
  rotate(angular_velocity);
  rotation_in_progress = true;

  // Sleep for the calculated rotation time
  ros::Duration(rotation_time).sleep();

  // Stop the rotation
  rotate(0.0);
  rotation_in_progress = false;

  res.success = "Rotation completed.";
  return true;
}

int main(int argc, char **argv) {
  ros::init(argc, argv, "control_robot");
  ros::NodeHandle nh;

  vel_pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 10);
  ros::ServiceServer service =
      nh.advertiseService("rotate_robot", rotateRobotCallback);

  ROS_INFO("Waiting for /rotate_robot service calls...");
  ros::spin();

  return 0;
}