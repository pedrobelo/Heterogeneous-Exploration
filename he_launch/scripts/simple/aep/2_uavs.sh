#!/bin/sh

rosservice call /gazebo/delete_model "model_name: 'iris_1'" & rosservice call /gazebo/delete_model "model_name: 'iris_2'";

roslaunch he_launch uavs_octomap.launch xc:=20 yc:=0 xl:=0 yl:=0 