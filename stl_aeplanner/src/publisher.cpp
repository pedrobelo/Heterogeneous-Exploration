#include <stl_aeplanner/multi_robot_collision.h>

class pubCollision
{
public:
	pubCollision(const ros::NodeHandle &n);
	bool block_path_service(stl_aeplanner_msgs::add_line_segment::Request &req, stl_aeplanner_msgs::add_line_segment::Response &res);
	void pubF();

	ros::Publisher pub;
	stl_aeplanner_msgs::line_segment msg_;
	ros::NodeHandle n_;
	ros::ServiceServer srv;
	bool init = false;
};

pubCollision::pubCollision(const ros::NodeHandle &n) : n_(n) {
	std::string robot_name;

	n_.getParam("robot_name", robot_name);
	ROS_ERROR_STREAM(robot_name + "/block_path");
	pub = n_.advertise<stl_aeplanner_msgs::line_segment>("/occupied", 1000);
	srv = n_.advertiseService(robot_name + "/block_path", &pubCollision::block_path_service, this);
}

bool pubCollision::block_path_service(stl_aeplanner_msgs::add_line_segment::Request &req, stl_aeplanner_msgs::add_line_segment::Response &res) {
	msg_.pt1.x = req.pt1.x; msg_.pt1.y = req.pt1.y; msg_.pt1.z = req.pt1.z;
	msg_.pt2.x = req.pt2.x; msg_.pt2.y = req.pt2.y; msg_.pt2.z = req.pt2.z;
	init = true;

	return true;
}

void pubCollision::pubF() {
	msg_.header.stamp = ros::Time::now();

	pub.publish(msg_);
}


int main(int argc, char **argv)
{
	ros::init(argc, argv, "pubCollision");
	ros::NodeHandle n("~");
	ros::Rate loop_rate(50);

	pubCollision pc(n);

	while (ros::ok())
	{
		ros::spinOnce();

		if(pc.init){
			pc.pubF();
		}

		loop_rate.sleep();
	}

	ros::spin();

	return 0;
}
