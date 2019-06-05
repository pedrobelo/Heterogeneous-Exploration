#include <ros/ros.h>
#include <octomap/octomap.h>
#include <octomap_msgs/conversions.h>
#include <evaluation/start.h>


#define INTERVAL 0.1

//init resolution
class evaluation_octomap
{
public:
	evaluation_octomap();
	~evaluation_octomap();
private:
	ros::NodeHandle nhPrivate_;
	std::shared_ptr<octomap::OcTree> ot_;
	std::ofstream file;
	double uncertainty1, uncertainty2;
	int intervals[int(floor(1/INTERVAL))];
	double x_max, y_max, z_max, resolution;
	double x_min, y_min, z_min;
	ros::Subscriber sub;
	ros::ServiceServer service;

	void octomapCallback(const octomap_msgs::Octomap& msg);
	void writeToFile();
	bool start(evaluation::start::Request &req, evaluation::start::Response &res);
};

evaluation_octomap::evaluation_octomap() : nhPrivate_("~"),
						   file("/home/belo/Desktop/"){
	std::string fileName;
	nhPrivate_.getParam("file", fileName);

	file.open(fileName, std::ofstream::out);

	nhPrivate_.getParam("x_max", x_max);
	nhPrivate_.getParam("y_max", y_max);
	nhPrivate_.getParam("z_max", z_max);

	nhPrivate_.getParam("x_min", x_min);
	nhPrivate_.getParam("y_min", y_min);
	nhPrivate_.getParam("z_min", z_min);

	service = nhPrivate_.advertiseService("start_recording", &evaluation_octomap::start, this);
}

evaluation_octomap::~evaluation_octomap() {
	//file << "\n";
	ROS_INFO("File closed");
	file.close();
}

bool evaluation_octomap::start(evaluation::start::Request &req, evaluation::start::Response &res) {
	std::string octomapTopic;
	nhPrivate_.getParam("octomapTopic", octomapTopic);
	sub = nhPrivate_.subscribe(octomapTopic, 1000, &evaluation_octomap::octomapCallback, this);

	return true;
}

void evaluation_octomap::octomapCallback(const octomap_msgs::Octomap& msg)
{
	ROS_INFO("Message received");
	resolution = msg.resolution;
	octomap::AbstractOcTree* aot = octomap_msgs::msgToMap(msg);
	octomap::OcTree* ot = (octomap::OcTree*)aot;
	ot_ = std::make_shared<octomap::OcTree>(*ot);
	delete ot;

	if(ot_) {
		file << msg.header.stamp << " ";
		evaluation_octomap::writeToFile();
	}
}

void evaluation_octomap::writeToFile() {
	double x, y, z, uncertainty;
	int i=0;
	uncertainty1 = 0;
	uncertainty2 = 0;
	for (int j = 0; j <= 1/INTERVAL; ++j)
	{
		intervals[j] = 0;
	}
	std::shared_ptr<octomap::OcTree> ot = ot_;
	for (x = x_min; x < x_max; x += resolution) {
		for (y = y_min; y < y_max; y += resolution) {
			for (z = z_min; z < z_max; z += resolution) {
				octomap::point3d query(x, y, z);
				octomap::OcTreeNode* result = ot->search(query);

				if(result) {
					uncertainty = pow(0.5 - std::abs(0.5-result->getOccupancy()), 2);
					uncertainty1 += uncertainty;
					uncertainty2 += uncertainty;
					intervals[int(floor(result->getOccupancy()/INTERVAL))]++;
					i++;
				}
				else {
					uncertainty = pow(0.5, 2);
					uncertainty1 += uncertainty;
				}
			}
		}
	}

	file << uncertainty1 << " " << uncertainty2;
	for (int j = 0; j <= 1/INTERVAL; ++j) {
		file << " " << intervals[j];
	}
	file << "\n";
}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "evaluation_octomap");
	
	evaluation_octomap evaluation_octomap_;

	ros::spin();
	return 0;
}