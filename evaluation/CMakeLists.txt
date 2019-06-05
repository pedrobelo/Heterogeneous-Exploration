cmake_minimum_required(VERSION 2.8.3)
project(evaluation)

add_definitions(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
	roscpp
	message_generation
)

find_package(cmake_modules REQUIRED)
find_package(octomap REQUIRED)

add_service_files(
	DIRECTORY srv
	FILES start.srv
)

generate_messages(
	DEPENDENCIES
	std_msgs
)

catkin_package(
  INCLUDE_DIRS ${OCTOMAP_INCLUDE_DIRS} ${catkin_INCLUDE_DIRS}
  LIBRARIES ${catkin_LIBRARIES} ${OCTOMAP_LIBRARIES}
  CATKIN_DEPENDS roscpp message_runtime
)

include_directories(
  ${catkin_INCLUDE_DIRS}
  ${OCTOMAP_INCLUDE_DIRS}
)

add_executable(evaluation src/evaluation.cpp)
target_link_libraries( evaluation ${catkin_LIBRARIES} ${OCTOMAP_LIBRARIES})
