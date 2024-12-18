#include "mavros_offboard_controller.hpp"
#include <cstdlib>
#include <iostream>
using std::placeholders::_1;

//============================================================================================
//CONSTRUCTOR AND DESTRUCTOR
mavros_offboard_controller::mavros_offboard_controller() : Node("mavros_offboard_controller"), publishing_frequency(20) {
  RCLCPP_INFO(rclcpp::get_logger("mavros_offboard_controller"), "Controller initialized");

  // Publishers, subscribers and clients.
  state_subscriber = this->create_subscription<mavros_msgs::msg::State>( "mavros/state", 10, std::bind(&mavros_offboard_controller::state_callback, this, _1));			
  odometry_subscriber = this->create_subscription<nav_msgs::msg::Odometry>( "mavros/local_position/odom", 10, std::bind(&mavros_offboard_controller::odometry_callback, this, _1));

  local_position_publisher = this->create_publisher<geometry_msgs::msg::PoseStamped>( "mavros/setpoint_position/local", 10);

  arming_client = this->create_client<mavros_msgs::srv::CommandBool>("mavros/cmd/arming");	
  mode_setting_client = this->create_client<mavros_msgs::srv::SetMode>("mavros/set_mode");
  timer = this->create_wall_timer(std::chrono::milliseconds(publishing_frequency),std::bind(&mavros_offboard_controller::local_position_callback, this));
}

mavros_offboard_controller::~mavros_offboard_controller() {
  RCLCPP_INFO(rclcpp::get_logger("mavros_offboard_controller"), "Controller destroyed");
}


//============================================================================================
//CALLBACKS
void mavros_offboard_controller::state_callback(const mavros_msgs::msg::State& msg)
{
  current_state = msg;

  if(!msg.armed)
  {
    std::string command = "ros2 service call /mavros/cmd/arming mavros_msgs/srv/CommandBool \"{value: True}\"";//TODO: this is scuffed, check if works.
    system(command.c_str());
  }

  if(msg.mode != "OFFBOARD")
  {
    std::string command = "ros2 service call /mavros/set_mode mavros_msgs/srv/SetMode \"{custom_mode: \"OFFBOARD\"}\"";//TODO: this is scuffed, check if works.
    system(command.c_str());
  }

}

void mavros_offboard_controller::odometry_callback(const nav_msgs::msg::Odometry& msg)
{
  current_position = msg;
}

void mavros_offboard_controller::local_position_callback()//TODO: This is 100% not how this is supposed to work, need to update later.
{
  geometry_msgs::msg::PoseStamped position_message;
  current_position.pose.pose.position;

  position_message.header.stamp = this->get_clock()->now();

  position_message.pose = current_position.pose.pose;

  local_position_publisher->publish(position_message);
}