#!/bin/sh

echo $1
echo $2
echo $3
echo $4
echo $5
echo $6
echo $7


gnome-terminal --tab -- roslaunch he_launch gazebo.launch world:=simple2 &

sleep 5;

gnome-terminal --tab -- rosservice call /gazebo/delete_model "model_name: 'iris_1'" & rosservice call /gazebo/delete_model "model_name: 'iris_2'";
gnome-terminal --tab -- roslaunch he_launch uavs_octomap.launch xc:=$1 yc:=$2 xl:=$3 yl:=$4 file_name:=$5 &

sleep 5;

gnome-terminal --tab -- roslaunch he_launch exp.launch path:=$6 &

sleep 5;

gnome-terminal --tab -- rosrun mavros mavcmd -n iris_2/mavros long 511 32 5000 0 0 0 0 0;
gnome-terminal --tab -- rosservice call /iris_2/mavros/cmd/arming '{value: true}';

gnome-terminal --tab -- rosrun mavros mavcmd -n iris_1/mavros long 511 32 5000 0 0 0 0 0;
gnome-terminal --tab -- rosservice call /iris_1/mavros/cmd/arming '{value: true}';

gnome-terminal --tab -- rosservice call /eval/start_recording
gnome-terminal --tab -- rosservice call /iris_2/mavros/set_mode '{base_mode: 0, custom_mode: 'offboard'}';
gnome-terminal --tab -- rosservice call /iris_1/mavros/set_mode '{base_mode: 0, custom_mode: 'offboard'}';

sleep $7;

killall roslaunch