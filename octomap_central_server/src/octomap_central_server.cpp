#include <octomap_server/OctomapServer.h>

using namespace octomap_server;

class octomap_central_server : public OctomapServer {
public:
	octomap_central_server(ros::NodeHandle private_nh_);
protected:
	ros::Subscriber lidar_sub, rgbd_sub;
	double rgbd_maxRange, rgbd_probHit, rgbd_probMiss, rgbd_thresMin, rgbd_thresMax, lidar_maxRange, lidar_probHit, lidar_probMiss, lidar_thresMin, lidar_thresMax;
	void rgbd_Callback(const sensor_msgs::PointCloud2::ConstPtr& cloud);
	void lidar_Callback(const sensor_msgs::PointCloud2::ConstPtr& cloud);
};

octomap_central_server::octomap_central_server(ros::NodeHandle private_nh_) {
	ros::NodeHandle private_nh(private_nh_);

	private_nh.param("rgbd/max_range", rgbd_maxRange, rgbd_maxRange);
	private_nh.param("rgbd/hit", rgbd_probHit, 0.7);
	private_nh.param("rgbd/miss", rgbd_probMiss, 0.4);
	private_nh.param("rgbd/min", rgbd_thresMin, 0.12);
	private_nh.param("rgbd/max", rgbd_thresMax, 0.97);

	private_nh.param("lidar/max_range", lidar_maxRange, lidar_maxRange);
	private_nh.param("lidar/hit", lidar_probHit, 0.7);
	private_nh.param("lidar/miss", lidar_probMiss, 0.4);
	private_nh.param("lidar/min", lidar_thresMin, 0.12);
	private_nh.param("lidar/max", lidar_thresMax, 0.97);


	lidar_sub = private_nh.subscribe("lidar_pc", 10, &octomap_central_server::lidar_Callback, this);
	rgbd_sub = private_nh.subscribe("rgbd_pc", 10, &octomap_central_server::rgbd_Callback, this);

}

void octomap_central_server::rgbd_Callback(const sensor_msgs::PointCloud2::ConstPtr& cloud){
	m_maxRange = rgbd_maxRange;
	m_octree->setProbHit(rgbd_probHit);
	m_octree->setProbMiss(rgbd_probMiss);
	m_octree->setClampingThresMin(rgbd_thresMin);
	m_octree->setClampingThresMax(rgbd_thresMax);

	insertCloudCallback(cloud);
}

void octomap_central_server::lidar_Callback(const sensor_msgs::PointCloud2::ConstPtr& cloud){
	m_maxRange = lidar_maxRange;
	m_octree->setProbHit(lidar_probHit);
	m_octree->setProbMiss(lidar_probMiss);
	m_octree->setClampingThresMin(lidar_thresMin);
	m_octree->setClampingThresMax(lidar_thresMax);

	insertCloudCallback(cloud);
}


int main(int argc, char** argv){
	ros::init(argc, argv, "octomap_central_server");
	const ros::NodeHandle& private_nh = ros::NodeHandle("~");
	std::string mapFilename(""), mapFilenameParam("");

	if (argc > 2 || (argc == 2 && std::string(argv[1]) == "-h")){
		exit(-1);
	}

	octomap_central_server server(private_nh);
	ros::spinOnce();

	if (argc == 2){
		mapFilename = std::string(argv[1]);
	}

	if (private_nh.getParam("map_file", mapFilenameParam)) {
		if (mapFilename != "") {
			ROS_WARN("map_file is specified by the argument '%s' and rosparam '%s'. now loads '%s'", mapFilename.c_str(), mapFilenameParam.c_str(), mapFilename.c_str());
		} else {
			mapFilename = mapFilenameParam;
		}
	}

	if (mapFilename != "") {
		if (!server.openFile(mapFilename)){
			ROS_ERROR("Could not open file %s", mapFilename.c_str());
			exit(1);
		}
	}

	ros::spin();

	return 0;
}

