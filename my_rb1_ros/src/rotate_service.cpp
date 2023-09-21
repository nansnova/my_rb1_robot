#include <geometry_msgs/Twist.h>
#include <my_rb1_ros/Rotate.h> // Import your custom service message
#include <nav_msgs/Odometry.h>
#include <ros/ros.h>
#include <tf/transform_datatypes.h>

double roll = 0.0;
double pitch = 0.0;
double yaw = 0.0;
double target_angle_degrees = 0.0; // Initialize with 0 degrees initially
double target_angle_radians = 0.0; // Store target angle in radians
double kP = 0.5;
bool rotation_requested = false;

ros::Publisher pub;
geometry_msgs::Twist command;

void get_rotation(const nav_msgs::Odometry::ConstPtr &msg) {
  if (!rotation_requested) {
    // Set the initial angular velocity to 0 only if rotation hasn't been
    // requested
    command.angular.z = 0.0;
  } else {
    tf::Quaternion orientation_q;
    tf::quaternionMsgToTF(msg->pose.pose.orientation, orientation_q);
    tf::Matrix3x3 mat(orientation_q);
    mat.getRPY(roll, pitch, yaw);
    ROS_INFO("Yaw: %f radians", yaw);

    double error = target_angle_radians - yaw;

    // Normalize error to the range of -pi to pi
    while (error > M_PI) {
      error -= 2 * M_PI;
    }
    while (error < -M_PI) {
      error += 2 * M_PI;
    }

    // Ensure the robot always takes the shorter path
    if (error > M_PI) {
      error = error - 2 * M_PI;
    } else if (error < -M_PI) {
      error = error + 2 * M_PI;
    }

    command.angular.z = kP * error;

    // Check if the robot has reached the target angle
    if (fabs(error) < 0.01) {
      target_angle_degrees = 0.0; // Reset the target angle to 0 degrees
      target_angle_radians = 0.0;
      rotation_requested = false; // Reset the rotation request flag
    }
  }
  pub.publish(command);
}

bool rotateRobotCallback(my_rb1_ros::Rotate::Request &req,
                         my_rb1_ros::Rotate::Response &res) {
  // Get the requested degrees from the service request
  double requested_degrees = req.degrees;

  // Calculate the target angle in radians
  target_angle_radians += requested_degrees * M_PI / 180.0;

  // Set the rotation request flag to true
  rotation_requested = true;

  ROS_INFO("Rotation requested: %.2f degrees", requested_degrees);

  res.success = "Rotation requested.";
  return true;
}

int main(int argc, char **argv) {
  ros::init(argc, argv, "control_robot");
  ros::NodeHandle nh;

  ros::Subscriber sub = nh.subscribe("/odom", 1, get_rotation);
  pub = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 1);

  // Create a service "/rotate_robot" that allows initiating the rotation
  ros::ServiceServer service =
      nh.advertiseService("/rotate_robot", rotateRobotCallback);

  ROS_INFO("Waiting for /rotate_robot service calls...");

  ros::spin();

  return 0;
}
