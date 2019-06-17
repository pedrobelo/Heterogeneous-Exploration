#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <sensor_msgs/PointCloud2.h>
#include <std_msgs/Header.h>
#include <pcl_ros/point_cloud.h>
#include <pcl_ros/transforms.h>
#include <pcl_conversions/pcl_conversions.h>

//distance_threshold param

struct coordinates {
	double x;
	double y;
	double z;
	ros::Time time;
};

class pc_handle
{
public:
	pc_handle();

private:
	typedef pcl::PointCloud<pcl::PointXYZ> PointCloud;

	coordinates coords1, coords2;
	tf::TransformListener listener;

	ros::Publisher lidarPublisher_, rgbdPublisher_;
	ros::Subscriber lidarSubscriber_, rgbdSubscriber_;

	ros::NodeHandle nh_private;

	std::vector<bool> keep;

	double distance_threshold;

	void lidarCallback(const sensor_msgs::PointCloud2::ConstPtr &in);
	void rgbdCallback(const sensor_msgs::PointCloud2::ConstPtr &in);
	void filter_all(const PointCloud& data_in);
	void tfToCoords(std::string world_frame, std::string local_frame, coordinates &coords);
	void filter_single(const PointCloud& data_in, coordinates coords);
	void filter_single(const PointCloud& data_in, std::string local_frame);
	void truncate(const PointCloud& data_in, PointCloud& data_out);
};

pc_handle::pc_handle() : nh_private("~"){
	keep.resize(0);

	lidarPublisher_ = nh_private.advertise<sensor_msgs::PointCloud2>("/lidar_pc_filtered", 10);
	lidarSubscriber_ = nh_private.subscribe("/lidar_pc", 100, &pc_handle::lidarCallback, this);
	rgbdPublisher_ = nh_private.advertise<sensor_msgs::PointCloud2>("/rgbd_pc_filtered", 10);
	rgbdSubscriber_ = nh_private.subscribe("/rgbd_pc", 100, &pc_handle::rgbdCallback, this);


	nh_private.getParam("distance_threshold", distance_threshold);
}


void pc_handle::lidarCallback(const sensor_msgs::PointCloud2::ConstPtr &in) 
{
		sensor_msgs::PointCloud2 out_msg;
		typename PointCloud::Ptr cloud(new PointCloud);
		PointCloud cloud_tranformed, out;
		
		pcl::fromROSMsg(*in, *cloud);

		filter_all(*cloud);
		truncate(*cloud, out);

		pcl::toROSMsg(out, out_msg);
		out_msg.header = in->header;

		lidarPublisher_.publish(out_msg);
}

void pc_handle::rgbdCallback(const sensor_msgs::PointCloud2::ConstPtr &in) 
{
		sensor_msgs::PointCloud2 out_msg;
		typename PointCloud::Ptr cloud(new PointCloud);
		PointCloud cloud_tranformed, out;
		
		pcl::fromROSMsg(*in, *cloud);

		filter_all(*cloud);
		truncate(*cloud, out);

		pcl::toROSMsg(out, out_msg);
		out_msg.header = in->header;

		rgbdPublisher_.publish(out_msg);
}


void pc_handle::filter_all(const PointCloud& data_in) {
	const unsigned int np = data_in.points.size();

	if(np > keep.size()) {
		keep.resize((int)np);
	}

	for (int i = 0 ; i < (int)np ; ++i)
		keep[i] = true;

	filter_single(data_in, "iris_1/base_link");
	filter_single(data_in, "iris_2/base_link");
}

void pc_handle::tfToCoords(std::string world_frame, std::string local_frame, coordinates &coords) {
	tf::StampedTransform transform;
	listener.lookupTransform(world_frame, local_frame, ros::Time(0), transform);
	coords.x = transform.getOrigin().x();
	coords.y = transform.getOrigin().y();
	coords.z = transform.getOrigin().z();
	coords.time = transform.stamp_;
}

void pc_handle::filter_single(const PointCloud& data_in, coordinates coords) {
	const unsigned int np = data_in.points.size();
	double distance;

	if(ros::Time::now().toSec() - coords.time.toSec() >= 0.1) {
		ROS_INFO("Coordinates are too old");
		return;
	}

	for (int i = 0 ; i < (int)np ; ++i) {
		if(keep[i]) {
			distance = std::pow(data_in.points[i].x-coords.x, 2);
			distance += std::pow(data_in.points[i].y-coords.y, 2);
			distance += std::pow(data_in.points[i].z-coords.z, 2);
			distance = std::sqrt(distance);

			//ROS_INFO("Coordinates: x=%f, y=%f, z=%f", data_in.points[i].x, data_in.points[i].y, data_in.points[i].z);

			if(distance <= distance_threshold)
				keep[i] = false;
		}
	}
}

void pc_handle::filter_single(const PointCloud& data_in, std::string local_frame) {
	coordinates coords_t, coords_it;

	ros::Time stamp;
	pcl_conversions::fromPCL(data_in.header.stamp, stamp);

	tf::StampedTransform transform;

	try {
		listener.waitForTransform(data_in.header.frame_id, local_frame, stamp, ros::Duration(0.1));
		listener.lookupTransform(data_in.header.frame_id, local_frame, stamp, transform);
	} catch (tf::TransformException ex) {
		ROS_ERROR("%s",ex.what());
	}

	coords_it.x = transform.getOrigin().x();
	coords_it.y = transform.getOrigin().y();
	coords_it.z = transform.getOrigin().z();

	const unsigned int np = data_in.points.size();
	double distance;

	for (int i = 0 ; i < (int)np ; ++i) {
		if(keep[i]) {
			distance = std::sqrt(std::pow(data_in.points[i].x-coords_it.x, 2) + std::pow(data_in.points[i].y-coords_it.y, 2) + std::pow(data_in.points[i].z-coords_it.z, 2));

			if(distance <= distance_threshold)
				keep[i] = false;
			else if(data_in.header.frame_id.substr(0, 6) != local_frame.substr(0, 6)) {
				double max_angle, drone_distance;
				drone_distance = std::sqrt(std::pow(coords_it.x, 2) + std::pow(coords_it.y, 2) + std::pow(coords_it.z, 2));
				max_angle = std::atan(distance_threshold/drone_distance);

				double p12, p13, p23;
				p12 = drone_distance;
				p13 = std::sqrt(std::pow(data_in.points[i].x, 2) + std::pow(data_in.points[i].y, 2) + std::pow(data_in.points[i].z, 2));
				p23 = std::sqrt(std::pow(coords_it.x-data_in.points[i].x, 2) + std::pow(coords_it.y-data_in.points[i].y, 2) + std::pow(coords_it.z-data_in.points[i].z, 2));

				double angle = std::acos((std::pow(p12, 2) + std::pow(p13, 2) - std::pow(p23, 2)) / (2*p12*p13));

				if(angle < max_angle && p13 > drone_distance)
					keep[i] = false;
			}
		}
	}
}

void pc_handle::truncate(const PointCloud& data_in, PointCloud& data_out) {
	const unsigned int np = data_in.points.size();

    // fill in output data with points that are NOT on the robot
    data_out.header = data_in.header;	  
	
    data_out.points.resize(0);
    data_out.points.reserve(np);
    for (unsigned int i = 0 ; i < np ; ++i)
		if (keep[i])
			data_out.points.push_back(data_in.points[i]);
}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "pc_handle");

	pc_handle pc_handle_;

	ros::spin();
	return 0;
}


