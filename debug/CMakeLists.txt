cmake_minimum_required(VERSION 2.8.3)
project(debug)

add_definitions(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
	roscpp
	geometry_msgs
	message_generation
)

find_package(Eigen3 REQUIRED)
find_package(cmake_modules REQUIRED)
find_package(octomap REQUIRED)

catkin_package(
  INCLUDE_DIRS ${Eigen_INCLUDE_DIRS} ${OCTOMAP_INCLUDE_DIRS} ${catkin_INCLUDE_DIRS}
  LIBRARIES ${catkin_LIBRARIES} ${OCTOMAP_LIBRARIES}
  CATKIN_DEPENDS roscpp message_runtime geometry_msgs
)

include_directories(
  ${catkin_INCLUDE_DIRS}
  ${OCTOMAP_INCLUDE_DIRS}
  ${Eigen_INCLUDE_DIRS}
)

add_executable(debug src/debug.cpp)
target_link_libraries( debug ${catkin_LIBRARIES} ${OCTOMAP_LIBRARIES})
