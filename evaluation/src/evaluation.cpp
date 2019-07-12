#include <ros/ros.h>
#include <octomap/octomap.h>
#include <octomap_msgs/conversions.h>
#include <evaluation/start.h>

#include <tf/transform_listener.h>
#include <tf/transform_datatypes.h>

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
	double uncertainty_all, uncertainty_occupied, uncertainty_free, uncertainty_explored;
	int intervals[int(floor(1/INTERVAL))];
	double x_max, y_max, z_max, resolution;
	double x_min, y_min, z_min;
	ros::Subscriber sub;
	ros::ServiceServer service, service2;
	tf::StampedTransform transform;

	void octomapCallback(const octomap_msgs::Octomap& msg);
	void writeToFile();
	bool start(evaluation::start::Request &req, evaluation::start::Response &res);
	bool writeOctomapToFile(evaluation::start::Request &req, evaluation::start::Response &res);
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
	service2 = nhPrivate_.advertiseService("print", &evaluation_octomap::writeOctomapToFile, this);
}

evaluation_octomap::~evaluation_octomap() {
	//file << "\n";
	ROS_INFO("File closed");
	file.close();
}

bool evaluation_octomap::start(evaluation::start::Request &req, evaluation::start::Response &res) {
	std::string octomapTopic;
	nhPrivate_.getParam("octomapTopic", octomapTopic);
	sub = nhPrivate_.subscribe(octomapTopic, 100, &evaluation_octomap::octomapCallback, this);

	return true;
}

void evaluation_octomap::octomapCallback(const octomap_msgs::Octomap& msg)
{
	static bool init=true;

	if(init) {
		// request the transform between the two frames
		tf::TransformListener listener;

		// the time to query the server is the stamp of the incoming message
		ros::Time time = msg.header.stamp;

		// frame of the incoming message
		std::string frame = msg.header.frame_id;

		try{
			listener.waitForTransform(frame, "world", time, ros::Duration(3.0));
			listener.lookupTransform(frame, "world", time, transform);
		}
		catch (tf::TransformException ex) {
			ROS_WARN("Base to camera transform unavailable %s", ex.what());
		}
		init = false;
	}

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

	uncertainty_all=0;
	uncertainty_explored=0;
	uncertainty_free=0;
	uncertainty_occupied=0;

	int all_cnt=0, explored_cnt=0, free_cnt=0, occupied_cnt=0;

	for (int j = 0; j <= 1/INTERVAL; ++j)
	{
		intervals[j] = 0;
	}
	std::shared_ptr<octomap::OcTree> ot = ot_;
	for (x = x_min; x < x_max; x += resolution) {
		for (y = y_min; y < y_max; y += resolution) {
			for (z = z_min; z < z_max; z += resolution) {
				tf::Vector3 point(x, y, z);
				point = transform*point;
				octomap::point3d query(point.getX(), point.getY(), point.getZ());
				octomap::OcTreeNode* result = ot->search(query);

				if(result) {
					uncertainty = pow(0.5 - std::abs(0.5-result->getOccupancy()), 2);
					uncertainty_all += uncertainty;
					uncertainty_explored += uncertainty;
					intervals[int(floor(result->getOccupancy()/INTERVAL))]++;
					
					if(result->getOccupancy() > 0.5) {
						uncertainty_occupied += uncertainty;
						occupied_cnt++;
					}
					if(result->getOccupancy() < 0.5) {
						uncertainty_free += uncertainty;
						free_cnt++;
					}

					explored_cnt++;
					all_cnt++;
				}
				else {
					uncertainty = pow(0.5, 2);
					uncertainty_all += uncertainty;
					all_cnt++;
				}
			}
		}
	}
	ROS_WARN_STREAM(uncertainty_explored << " " << explored_cnt);
	file << uncertainty_all << " " << uncertainty_all/all_cnt << " " << uncertainty_explored/explored_cnt << " " << uncertainty_occupied/occupied_cnt << " " << uncertainty_free/free_cnt;
	for (int j = 0; j <= 1/INTERVAL; ++j) {
		file << " " << intervals[j];
	}
	file << "\n";
}

bool evaluation_octomap::writeOctomapToFile(evaluation::start::Request &req, evaluation::start::Response &res) {
	if(ot_) {
		double x, y, z;
		file.open("/home/pedro/Desktop/histogram.txt", std::ofstream::out);
		std::shared_ptr<octomap::OcTree> ot = ot_;
		for (x = x_min; x < x_max; x += resolution) {
			for (y = y_min; y < y_max; y += resolution) {
				for (z = z_min; z < z_max; z += resolution) {
					octomap::point3d query(x, y, z);
					octomap::OcTreeNode* result = ot->search(query);

					if(result) {
						if(result->getOccupancy()*100 > 50) {
							file << result->getOccupancy()*100 << "\n";
						}
					}
				}
			}
		}

		file.close();
	}

	return true;
}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "evaluation_octomap");
	
	evaluation_octomap evaluation_octomap_;

	ros::spin();
	return 0;
}