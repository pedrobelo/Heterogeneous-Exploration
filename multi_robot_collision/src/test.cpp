#include <multi_robot_collision/multi_robot_collision.h>


int main(int argc, char **argv)
{
	ros::init(argc, argv, "pubCollision");
	ros::NodeHandle n("~");
	ros::Rate loop_rate(50);

	multi_robot_collision_node mrcn(n);
	mrcn.visualization_ = true;

	Eigen::Vector3f pt1, pt2;

	std::cin >> pt1(0);
	std::cin >> pt1(1);
	std::cin >> pt1(2);

	std::cin >> pt2(0);
	std::cin >> pt2(1);
	std::cin >> pt2(2);

	ros::Duration(0.5).sleep();
	ros::spinOnce();

	ROS_ERROR_STREAM(mrcn.block_path(pt1, pt2, true));

	while (ros::ok())
	{
		ros::spinOnce();

		loop_rate.sleep();
	}

	ros::spin();

	return 0;
}
