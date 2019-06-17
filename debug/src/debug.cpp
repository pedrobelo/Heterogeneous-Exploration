#include <ros/ros.h>
#include <octomap/octomap.h>
#include <octomap_msgs/conversions.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <eigen3/Eigen/Dense>

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
	ROS_INFO("Message received");
	octomap::AbstractOcTree* aot = octomap_msgs::msgToMap(msg);
	octomap::OcTree* ot = (octomap::OcTree*)aot;
	ot_ = std::make_shared<octomap::OcTree>(*ot);
	delete ot;
}

void debug::clickCallback(const geometry_msgs::PoseWithCovarianceStamped& msg)
{
	file.open("/home/belo/Desktop/debug_histogram.txt", std::ofstream::out);

	std::shared_ptr<octomap::OcTree> ot = ot_;
	if(!ot_)
		return;
	// This function computes the gain
	double fov_y = 115, fov_p = 60;

	double dr = 0.05, dphi = 1, dtheta = 1;
	double dphi_rad = M_PI * dphi / 180.0f, dtheta_rad = M_PI * dtheta / 180.0f;
	double r;
	int phi, theta;
	double phi_rad, theta_rad;

	Eigen::Vector3d origin(msg.pose.pose.position.x, msg.pose.pose.position.y, 1.5);
	Eigen::Vector3d vec, dir;

	for (theta = -180; theta < 180; theta += dtheta)
	{
		theta_rad = M_PI * theta / 180.0f;
		for (phi = 90 - fov_p / 2; phi < 90 + fov_p / 2; phi += dphi)
		{
			phi_rad = M_PI * phi / 180.0f;

			for (r = 0; r < 7; r += dr)
			{
				vec[0] = msg.pose.pose.position.x + r * cos(theta_rad) * sin(phi_rad);
				vec[1] = msg.pose.pose.position.y + r * sin(theta_rad) * sin(phi_rad);
				vec[2] = 1.5 + r * cos(phi_rad);
				dir = vec - origin;

				octomap::point3d query(vec[0], vec[1], vec[2]);
				octomap::OcTreeNode* result = ot->search(query);

				Eigen::Vector4d v(vec[0], vec[1], vec[2], 0);
				if (result) {
					if(result->getOccupancy() > 0.5)
						file << " " << result->getOccupancy()*100;
				}
			}
		}
	}
	file.close();
}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "debug");
	
	debug debug_;

	ros::spin();
	return 0;
}