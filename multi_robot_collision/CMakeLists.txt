cmake_minimum_required(VERSION 2.8.3)
project(multi_robot_collision)

add_compile_options(-std=c++11)

find_package(catkin REQUIRED
	roscpp
	geometry_msgs
	tf
	tf2
	visualization_msgs
	std_msgs
	message_generation
)

find_package(cmake_modules REQUIRED)
find_package(Eigen3 REQUIRED)

add_message_files(
	DIRECTORY msg 
	FILES line_segment.msg
)

add_service_files(
	DIRECTORY srv
	FILES add_line_segment.srv
)

generate_messages(
	DEPENDENCIES
	geometry_msgs
	std_msgs
)

catkin_package(
	INCLUDE_DIRS ${Eigen_INCLUDE_DIRS} ${catkin_INCLUDE_DIRS}
	LIBRARIES ${catkin_LIBRARIES}
	CATKIN_DEPENDS message_runtime roscpp tf tf2 visualization_msgs
	DEPENDS Eigen
)

include_directories(
	${catkin_INCLUDE_DIRS}
	${Eigen_INCLUDE_DIRS}
)

add_executable(multi_robot_collision src/multi_robot_collision.cpp)
target_link_libraries(multi_robot_collision
	${catkin_LIBRARIES}
)
add_dependencies(multi_robot_collision ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
