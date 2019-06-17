#include <ros/ros.h>
#include <octomap/octomap.h>
#include <octomap_msgs/conversions.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <eigen3/Eigen/Dense>

#include <tf/transform_listener.h>
#include <tf/transform_datatypes.h>

#define INTERVAL 0.1

//init resolution
class debug
{
public:
	debug();

private:
	ros::NodeHandle nhPrivate_;
	std::shared_ptr<octomap::OcTree> ot_;
	std::ofstream file;

	ros::Subscriber sub_octo, sub_click;

	void octomapCallback(const octomap_msgs::Octomap& msg);
	void clickCallback(const geometry_msgs::PoseWithCovarianceStamped& msg);

};

debug::debug() : nhPrivate_("~") {

	std::string octomapTopic;
	nhPrivate_.getParam("octomapTopic", octomapTopic);
	sub_octo = nhPrivate_.subscribe(octomapTopic, 1000, &debug::octomapCallback, this);
	sub_click = nhPrivate_.subscribe("/initialpose", 1000, &debug::clickCallback, this);
}

void debug::octomapCallback(const octomap_msgs::Octomap& msg)
{
	octomap::AbstractOcTree* aot = octomap_msgs::msgToMap(msg);
	octomap::OcTree* ot = (octomap::OcTree*)aot;
	ot_ = std::make_shared<octomap::OcTree>(*ot);
	delete ot;
}

void debug::clickCallback(const geometry_msgs::PoseWithCovarianceStamped& msg)
{
	file.open("/home/pedro/Desktop/debug_histogram.txt");

	std::shared_ptr<octomap::OcTree> ot = ot_;
	if(!ot_)
		return;
	// This function computes the gain
	double fov_p = 60;

	double dr = 0.05, dphi = 1, dtheta = 1;
	double dphi_rad = M_PI * dphi / 180.0f, dtheta_rad = M_PI * dtheta / 180.0f;
	double r;
	int phi, theta;
	double phi_rad, theta_rad;

	
	tf::Vector3 point(msg.pose.pose.position.x, msg.pose.pose.position.y, 1.5);

	// request the transform between the two frames
	tf::TransformListener listener;
	tf::StampedTransform transform;

	// the time to query the server is the stamp of the incoming message
	ros::Time time = msg.header.stamp;

	// frame of the incoming message
	std::string frame = msg.header.frame_id;

	try{
	  listener.waitForTransform("/iris_1/map", frame, time, ros::Duration(3.0));
	  listener.lookupTransform("/iris_1/map", frame, time, transform);
	}
	catch (tf::TransformException ex) {
	  ROS_WARN("Base to camera transform unavailable %s", ex.what());
	}

	point = transform*point;

	Eigen::Vector3d origin(point.getX(), point.getY(), point.getZ());

	Eigen::Vector3d vec, dir;

	std::cout << "opened" << std::endl;

	for (theta = -180; theta < 180; theta += dtheta)
	{
		theta_rad = M_PI * theta / 180.0f;
		for (phi = 90 - fov_p / 2; phi < 90 + fov_p / 2; phi += dphi)
		{
			phi_rad = M_PI * phi / 180.0f;

			for (r = 0; r < 7; r += dr)
			{
				vec[0] = point.getX() + r * cos(theta_rad) * sin(phi_rad);
				vec[1] = point.getY() + r * sin(theta_rad) * sin(phi_rad);
				vec[2] = 1.5 + r * cos(phi_rad);
				dir = vec - origin;

				octomap::point3d query(vec[0], vec[1], vec[2]);
				octomap::OcTreeNode* result = ot->search(query);

				Eigen::Vector4d v(vec[0], vec[1], vec[2], 0);
				if (result) {
					if(result->getOccupancy() > 0.5) {
						file << result->getOccupancy()*100 << std::endl;
						break;
					}
				}
			}
		}
	}
	file.close();
	std::cout << "closed" << std::endl;

}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "debug");
	
	debug debug_;

	ros::spin();
	return 0;
}