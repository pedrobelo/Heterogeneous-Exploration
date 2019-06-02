cmake_minimum_required(VERSION 2.8.3)
project(octomap_central_server)

add_compile_options(-std=c++11)

find_package( PCL REQUIRED )
add_definitions(${PCL_DEFINITIONS})

find_package(catkin REQUIRED COMPONENTS roscpp tf sensor_msgs std_msgs pcl_ros pcl_conversions octomap_server)
find_package(octomap REQUIRED)

## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
	INCLUDE_DIRS ${catkin_INCLUDE_DIRS}
#  LIBRARIES octomap_central_server
	CATKIN_DEPENDS roscpp tf sensor_msgs std_msgs pcl_ros pcl_conversions octomap_server
#  DEPENDS system_lib
)

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
	${catkin_INCLUDE_DIRS}
	${OCTOMAP_INCLUDE_DIRS}
)


add_executable(pc_handle src/pc_handle.cpp)
target_link_libraries(pc_handle ${catkin_LIBRARIES})
add_dependencies(pc_handle ${catkin_EXPORTED_TARGETS})

add_executable(octomap_central_server src/octomap_central_server.cpp)
target_link_libraries(octomap_central_server ${catkin_LIBRARIES} ${OCTOMAP_LIBRARIES})
add_dependencies(octomap_central_server ${catkin_EXPORTED_TARGETS})
