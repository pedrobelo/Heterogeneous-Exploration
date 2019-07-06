#!/bin/sh

gnome-terminal --tab -- roslaunch he_launch gazebo.launch world:=$9 &

sleep 5;

gnome-terminal --tab -- rosservice call /gazebo/delete_model "model_name: 'iris_1'" & rosservice call /gazebo/delete_model "model_name: 'iris_2'";
gnome-terminal --tab -- roslaunch he_launch uavs_octomap.launch xc:=$2 yc:=$3 xl:=$4 yl:=$5 file_name:=$6 &

sleep 5;

gnome-terminal --tab -- roslaunch he_launch exp.launch path:=$7 &

sleep 15;

gnome-terminal --tab -- rosrun mavros mavcmd -n iris_2/mavros long 511 32 5000 0 0 0 0 0;
gnome-terminal --tab -- rosservice call /iris_2/mavros/cmd/arming '{value: true}';

gnome-terminal --tab -- rosrun mavros mavcmd -n iris_1/mavros long 511 32 5000 0 0 0 0 0;
gnome-terminal --tab -- rosservice call /iris_1/mavros/cmd/arming '{value: true}';

gnome-terminal --tab -- rosservice call /eval/start_recording;
gnome-terminal --tab -- rosservice call /iris_2/mavros/set_mode '{base_mode: 0, custom_mode: 'offboard'}';
gnome-terminal --tab -- rosservice call /iris_1/mavros/set_mode '{base_mode: 0, custom_mode: 'offboard'}';

sleep $8;

killall roslaunch;

sleep 20;