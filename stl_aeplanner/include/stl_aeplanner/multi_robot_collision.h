#ifndef MULTI_ROBOT_COLLISION_HPP
#define MULTI_ROBOT_COLLISION_HPP


#include <ros/ros.h>
#include <eigen3/Eigen/Dense>
#include <stl_aeplanner_msgs/add_line_segment.h>
#include <stl_aeplanner_msgs/line_segment.h>

#include <tf/transform_broadcaster.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

#include <visualization_msgs/MarkerArray.h>

#include <limits>


/*
This class keeps track of other robots. It is used to save planned paths with a time stamp
so that old paths are simply removed.

Each node sends it's own path periodically so that it is not removed from other robots
*/
class robot_tracker
{
public:
	robot_tracker(const Eigen::Vector3f &pt1, const Eigen::Vector3f &pt2, const ros::Time &stamp);
	bool update(const Eigen::Vector3f &pt1, const Eigen::Vector3f &pt2, const ros::Time &stamp);
	bool expired();

	//start and end point of the path(considered a straight line)
	Eigen::Vector3f pt1_, pt2_;

	//time stamp of when the path was received
	ros::Time stamp_;

	//maximum time a path is kept without any updates
	ros::Duration keep_alive_;	
};



class multi_robot_collision_node
{
public:
	multi_robot_collision_node(const ros::NodeHandle &n);
	void broadcaster();

	Eigen::Vector3f pt1_, pt2_;

	void remove();
	void pathCallback(const stl_aeplanner_msgs::line_segment::ConstPtr& msg);
	bool block_path(const Eigen::Vector3f &pt1, const Eigen::Vector3f &pt2, bool broadcast);
	bool free_space(const Eigen::Vector3f &pt1, const Eigen::Vector3f &pt2);
	float distance_between_two_lines(const Eigen::Vector3f &start1, const Eigen::Vector3f &start2, const Eigen::Vector3f &finish1, const Eigen::Vector3f &finish2, Eigen::Vector3f &pt1_seg, Eigen::Vector3f &pt2_seg);
	float distance_point_line_segment(const Eigen::Vector3f &pt1, const Eigen::Vector3f &pt2, const Eigen::Vector3f &point, Eigen::Vector3f &closest_pt);
	float distance_between_points(const Eigen::Vector3f &p1, const Eigen::Vector3f &p2);
	bool point_inside_line_segment(const Eigen::Vector3f &start, const Eigen::Vector3f &finish, const Eigen::Vector3f &point);
	float distance_between_line_segments(const Eigen::Vector3f &start1, const Eigen::Vector3f &start2, const Eigen::Vector3f &finish1, const Eigen::Vector3f &finish2, const int &mode);
	void visualization(const Eigen::Vector3f &pt1, const Eigen::Vector3f &pt2, int mode);

	//publish markers of the path/closest points between paths
	bool visualization_;

private:
	//vector with all the paths of other robots
	std::vector<robot_tracker> robot_tracker_;

	//minimum distance between robots' paths
	double distance_threshold_;

	//broadcast its own path or not
	bool broadcast;

	stl_aeplanner_msgs::add_line_segment msg_;
	ros::NodeHandle n_;
	ros::Subscriber sub;
	ros::Publisher marker_pub;
	ros::ServiceClient srvClient;
};

#endif