#!/bin/sh

#rosrun mavros mavcmd -n iris_1/mavros long 511 32 5000 0 0 0 0 0;
rosservice call /iris_1/mavros/cmd/arming '{value: true}';
rosservice call /iris_1/mavros/set_mode '{base_mode: 0, custom_mode: 'offboard'}';
rosservice call /eval/start_recording
