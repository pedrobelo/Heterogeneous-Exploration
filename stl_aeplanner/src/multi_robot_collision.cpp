#include <stl_aeplanner/multi_robot_collision.h>


robot_tracker::robot_tracker(const Eigen::Vector3f &pt1, const Eigen::Vector3f &pt2, const ros::Time &stamp) : keep_alive_(0.2) {
	pt1_ = pt1;
	pt2_ = pt2;
	stamp_ = stamp;
}

//Checks if the input path is the same of the one in this class
//if it is, updates the time stamp
bool robot_tracker::update(const Eigen::Vector3f &pt1, const Eigen::Vector3f &pt2, const ros::Time &stamp) {
	if(pt1 == pt1_ && pt2 == pt2_) {
		stamp_ = stamp;
		return true;
	}

	return false;
}

//returns true for paths that are too old (so that they can be removed). False otherwise
bool robot_tracker::expired() {
	ros::Duration difference = ros::Time::now() - stamp_;

	if(difference > keep_alive_)
		return true;
	return false;
}



multi_robot_collision_node::multi_robot_collision_node(const ros::NodeHandle &n) : n_(n) {
	broadcast = false;
	sub = n_.subscribe("/occupied", 1000, &multi_robot_collision_node::pathCallback, this);
	marker_pub = n_.advertise<visualization_msgs::MarkerArray>("visualization_marker", 1);

	if(!n_.getParam("distance_threshold", distance_threshold_))
		distance_threshold_ = 0.1;

	if(!n_.getParam("visualization", visualization_))
		visualization_ = 0.1;

	std::string robot_name;

	ros::param::get(ros::this_node::getNamespace() + "/he/robot_name", robot_name);

	ROS_ERROR_STREAM("/" + robot_name + "/block_path");
	srvClient = n_.serviceClient<stl_aeplanner_msgs::add_line_segment>("/" + robot_name + "/block_path");
}

void multi_robot_collision_node::remove() {
	//search for the path, if it already exists, and update it (update time stamp)
	for (int i = 0; i < robot_tracker_.size(); i++) {
		if(robot_tracker_[i].expired()) {
			robot_tracker_.erase(robot_tracker_.begin()+i);
			i--;
		}
	}
}

void multi_robot_collision_node::pathCallback(const stl_aeplanner_msgs::line_segment::ConstPtr& msg){
	Eigen::Vector3f pt1, pt2;

	pt1(0) = msg->pt1.x; pt1(1) = msg->pt1.y; pt1(2) = msg->pt1.z;
	pt2(0) = msg->pt2.x; pt2(1) = msg->pt2.y; pt2(2) = msg->pt2.z;

	//if it is own path, discard it
	if(pt1 == pt1_ && pt2 == pt2_)
		return;

	//search for the path, if it already exists, and update it (update time stamp)
	for (int i = 0; i < robot_tracker_.size(); i++) {
		if(robot_tracker_[i].update(pt1, pt2, msg->header.stamp))
			return;
	}
	
	//if path is not in the vector, add it
	robot_tracker rt(pt1, pt2, msg->header.stamp);
	robot_tracker_.push_back(rt);
}

bool multi_robot_collision_node::block_path(const Eigen::Vector3f &pt1, const Eigen::Vector3f &pt2, bool broadcast) {
	ROS_ERROR_STREAM("points");
	ROS_ERROR_STREAM(pt1);
	ROS_ERROR_STREAM(pt2);

	bool success;
	//if path is a point, stop broadcasting
	pt1_ = pt1;
	pt2_ = pt2;

	multi_robot_collision_node::remove();
	//check if received path is free and, if so, start broadcasting it
	success = multi_robot_collision_node::free_space(pt1, pt2);

	if(success) {
		msg_.request.pt1.x = pt1(0); msg_.request.pt1.y = pt1(1); msg_.request.pt1.z = pt1(2);
		msg_.request.pt2.x = pt2(0); msg_.request.pt2.y = pt2(1); msg_.request.pt2.z = pt2(2);
		if(!srvClient.call(msg_))
			ROS_ERROR_STREAM("Failed to call service block_path");
	}
	else
		return false;

	return true;
}

bool multi_robot_collision_node::free_space(const Eigen::Vector3f &pt1, const Eigen::Vector3f &pt2) {
	double distance;
	//iterate through all paths and see if own path doesn't intersect any of them
	for (int i = 0; i < robot_tracker_.size(); i++) {
		distance = multi_robot_collision_node::distance_between_line_segments(pt1, robot_tracker_[i].pt1_, pt2, robot_tracker_[i].pt2_, i+1);
		if(distance <= distance_threshold_)
			return false;
	}
	multi_robot_collision_node::visualization(pt1, pt2, 0);
	return true;
}


/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2010, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the copyright holder(s) nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id$
 *
 */
//distance between 2 lines defined by 2 points each
float multi_robot_collision_node::distance_between_two_lines(const Eigen::Vector3f &l1_pt1, const Eigen::Vector3f &l2_pt1, const Eigen::Vector3f &l1_pt2, const Eigen::Vector3f &l2_pt2, Eigen::Vector3f &l1_mid, Eigen::Vector3f &l2_mid) {
	// a = x2 - x1 = line_a[1] - line_a[0]
	Eigen::Vector3f u = l1_pt2 - l1_pt1;
	// b = x4 - x3 = line_b[1] - line_b[0]
	Eigen::Vector3f v = l2_pt2 - l2_pt1;
	// c = x2 - x3 = line_a[1] - line_b[0]
	Eigen::Vector3f w = l1_pt2 - l2_pt1;

	float a = u.dot (u);
	float b = u.dot (v);
	float c = v.dot (v);
	float d = u.dot (w);
	float e = v.dot (w);
	float denominator = a*c - b*b;
	float sc, tc;
	// Compute the line parameters of the two closest points
	if (denominator < 1e-5)          // The lines are almost parallel
	{
		sc = 0.0;
		tc = (b > c ? d / b : e / c);  // Use the largest denominator
	}
	else
	{
		sc = (b*e - c*d) / denominator;
		tc = (a*e - b*d) / denominator;
	}

	// Get the closest points
	l1_mid = Eigen::Vector3f::Zero ();
	l1_mid = l1_pt2 + sc * u;

	l2_mid = Eigen::Vector3f::Zero ();
	l2_mid = l2_pt1 + tc * v;

	if(multi_robot_collision_node::point_inside_line_segment(l1_pt1, l1_pt2, l1_mid) && multi_robot_collision_node::point_inside_line_segment(l2_pt1, l2_pt2, l2_mid)) {
		float distance = multi_robot_collision_node::distance_between_points(l1_mid, l2_mid);

		return distance;
	}
	return -1;

}

float multi_robot_collision_node::distance_point_line_segment(const Eigen::Vector3f &pt1, const Eigen::Vector3f &pt2, const Eigen::Vector3f &point, Eigen::Vector3f &closest_pt) {
	Eigen::Vector3f a, b;

	a = pt2 - pt1;
	b = point - pt1;

	float sqr_length = a.dot(a);
	float t = a.dot(b)/sqr_length;

	closest_pt = pt1 + t * a;
	if(multi_robot_collision_node::point_inside_line_segment(pt1, pt2, closest_pt)) {
		Eigen::Vector3f aux = closest_pt - point;
		return std::sqrt(aux.dot(aux));
	}
	return -1;
}

//distance between 2 points
float multi_robot_collision_node::distance_between_points(const Eigen::Vector3f &pt1, const Eigen::Vector3f &pt2) {
	return std::sqrt(std::pow(pt2(0)-pt1(0), 2) + std::pow(pt2(1)-pt1(1), 2) + std::pow(pt2(2)-pt1(2), 2));
}

// https://stackoverflow.com/questions/328107/how-can-you-determine-a-point-is-between-two-other-points-on-a-line-segment
//checks if points belongs to the line segment defined by pt1 and pt2
bool multi_robot_collision_node::point_inside_line_segment(const Eigen::Vector3f &pt1, const Eigen::Vector3f &pt2, const Eigen::Vector3f &point) {
	Eigen::Vector3f a, b, pt_aux;

	a = pt2 - pt1;
	b = point - pt1;

	pt_aux = a.cross(b);
	if(pt_aux.norm() > 1e-4)
		return false;

	if(a.dot(b) < 0)
		return false;

	if(a.dot(b) > a.squaredNorm())
		return false;

	return true;
}


float multi_robot_collision_node::distance_between_line_segments(const Eigen::Vector3f &l1_pt1, const Eigen::Vector3f &l2_pt1, const Eigen::Vector3f &l1_pt2, const Eigen::Vector3f &l2_pt2, const int &mode) {
	
	//points closest between 2 lines (may or may not be contained within line segment)
	Eigen::Vector3f pt_mid;

	//min_distance corresponds to the smallest distance between line segments
	float distance = -1, min_distance = -1;

	//points for visualization
	Eigen::Vector3f pt1_viz, pt2_viz;

	//gets closest points between two lines
	//returns -1 if the obtained points are not inside the line segments
	distance = multi_robot_collision_node::distance_between_two_lines(l1_pt1, l2_pt1, l1_pt2, l2_pt2, pt1_viz, pt2_viz);
	if(distance != -1) {
		if(visualization_)
			multi_robot_collision_node::visualization(pt1_viz, pt2_viz, mode);

		return distance;
	}

	distance = multi_robot_collision_node::distance_point_line_segment(l1_pt1, l1_pt2, l2_pt1, pt_mid);
	if(distance != -1 && (distance < min_distance || min_distance < 0)) {
		min_distance = distance;
		pt1_viz = l2_pt1; pt2_viz = pt_mid;
	}

	distance = multi_robot_collision_node::distance_point_line_segment(l1_pt1, l1_pt2, l2_pt2, pt_mid);
	if(distance != -1 && (distance < min_distance || min_distance < 0)) {
		min_distance = distance;
		pt1_viz = l2_pt2; pt2_viz = pt_mid;
	}

	distance = multi_robot_collision_node::distance_point_line_segment(l2_pt1, l2_pt2, l1_pt1, pt_mid);
	if(distance != -1 && (distance < min_distance || min_distance < 0)) {
		min_distance = distance;
		pt1_viz = l1_pt1; pt2_viz = pt_mid;
	}

	distance = multi_robot_collision_node::distance_point_line_segment(l2_pt1, l2_pt2, l1_pt2, pt_mid);
	if(distance != -1 && (distance < min_distance || min_distance < 0)) {
		min_distance = distance;
		pt1_viz = l1_pt2; pt2_viz = pt_mid;
	}

	distance = multi_robot_collision_node::distance_between_points(l1_pt1, l2_pt1);
	if(distance < min_distance || min_distance < 0) {
		min_distance = distance;
		pt1_viz = l1_pt1; pt2_viz = l2_pt1;
	}
	distance = multi_robot_collision_node::distance_between_points(l1_pt1, l2_pt2);
	if(distance < min_distance || min_distance < 0) {
		min_distance = distance;
		pt1_viz = l1_pt1; pt2_viz = l2_pt2;
	}
	distance = multi_robot_collision_node::distance_between_points(l1_pt2, l2_pt1);
	if(distance < min_distance || min_distance < 0) {
		min_distance = distance;
		pt1_viz = l1_pt2; pt2_viz = l2_pt1;
	}
	distance = multi_robot_collision_node::distance_between_points(l1_pt2, l2_pt2);
	if(distance < min_distance || min_distance < 0) {
		min_distance = distance;
		pt1_viz = l1_pt2;
		pt2_viz = l2_pt2;
	}

	if(visualization_) {
		multi_robot_collision_node::visualization(pt1_viz, pt2_viz, mode);
	}

	return min_distance;
}

void multi_robot_collision_node::visualization(const Eigen::Vector3f &pt1, const Eigen::Vector3f &pt2, int mode) {
	float r, g, b;


	//paths and shortest distances between paths are colour coded. green for the former, blue for the latter
	if(mode == 0) {
		r = 0; g = 0; b = 1;
	}
	else {
		r = 0; g = 1; b = 0;
	}

	visualization_msgs::MarkerArray markerarray;
	visualization_msgs::Marker marker;

	Eigen::Vector3f v = pt2-pt1;
	float projectionLength = std::sqrt(v(0) * v(0) + v(1) * v(1));
	float pitch = std::atan2(projectionLength, v(2));
	float yaw = std::atan2(v(1), v(0));
	float roll = 0;

	tf2::Quaternion myQuaternion;
	myQuaternion.setRPY( roll, pitch, yaw);

	myQuaternion.normalize();

	geometry_msgs::Quaternion quat_msg;
	quat_msg = tf2::toMsg(myQuaternion);

	marker.pose.orientation = quat_msg;

	//cylinder connecting start and finish point of the path
	marker.header.frame_id = "world";
	marker.header.stamp = ros::Time();
	marker.ns = ros::this_node::getNamespace();
	marker.id = 0 + mode*3;
	marker.type = visualization_msgs::Marker::CYLINDER;
	marker.action = visualization_msgs::Marker::ADD;
	marker.pose.position.x = (pt1(0)+pt2(0))/2;
	marker.pose.position.y = (pt1(1)+pt2(1))/2;
	marker.pose.position.z = (pt1(2)+pt2(2))/2;
	marker.scale.x = distance_threshold_;
	marker.scale.y = distance_threshold_;
	marker.scale.z = std::sqrt(pow(pt2(0)-pt1(0),2) + pow(pt2(1)-pt1(1),2) + pow(pt2(2)-pt1(2),2));
	marker.color.a = 0.5; // Don't forget to set the alpha!
	marker.color.r = r;
	marker.color.g = g;
	marker.color.b = b;

	markerarray.markers.push_back(marker);

	//starting point
	marker.header.frame_id = "world";
	marker.header.stamp = ros::Time();
	marker.ns = ros::this_node::getNamespace();
	marker.id = 1 + mode*3;
	marker.type = visualization_msgs::Marker::SPHERE;
	marker.action = visualization_msgs::Marker::ADD;
	marker.pose.position.x = pt1(0);
	marker.pose.position.y = pt1(1);
	marker.pose.position.z = pt1(2);
	marker.pose.orientation.x = 0.0;
	marker.pose.orientation.y = 0.0;
	marker.pose.orientation.z = 0.0;
	marker.pose.orientation.w = 1.0;
	marker.scale.x = distance_threshold_;
	marker.scale.y = distance_threshold_;
	marker.scale.z = distance_threshold_;
	marker.color.a = 0.5; // Don't forget to set the alpha!
	marker.color.r = r;
	marker.color.g = g;
	marker.color.b = b;

	markerarray.markers.push_back(marker);

	//finish point
	marker.header.frame_id = "world";
	marker.header.stamp = ros::Time();
	marker.ns = ros::this_node::getNamespace();
	marker.id = 2 + mode*3;
	marker.type = visualization_msgs::Marker::SPHERE;
	marker.action = visualization_msgs::Marker::ADD;
	marker.pose.position.x = pt2(0);
	marker.pose.position.y = pt2(1);
	marker.pose.position.z = pt2(2);
	marker.pose.orientation.x = 0.0;
	marker.pose.orientation.y = 0.0;
	marker.pose.orientation.z = 0.0;
	marker.pose.orientation.w = 1.0;
	marker.scale.x = distance_threshold_;
	marker.scale.y = distance_threshold_;
	marker.scale.z = distance_threshold_;
	marker.color.a = 0.5; // Don't forget to set the alpha!
	marker.color.r = r;
	marker.color.g = g;
	marker.color.b = b;

	markerarray.markers.push_back(marker);

	marker_pub.publish( markerarray );
}
