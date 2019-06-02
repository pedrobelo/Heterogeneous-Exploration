#!/bin/sh

rosservice call /gazebo/delete_model "model_name: 'iris_1'" & rosservice call /gazebo/delete_model "model_name: 'iris_2'";

roslaunch he_launch uavs.launch