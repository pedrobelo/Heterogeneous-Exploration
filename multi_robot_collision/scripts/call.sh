#!/bin/sh

rosservice call /1_ns/multi_robot_collision/block_path "{pt1: {x: 4, y: 3, z: 5}, pt2: {x: 5, y: 9, z: 2}}"
rosservice call /2_ns/multi_robot_collision/block_path "{pt1: {x: 1, y: 7, z: 4}, pt2: {x: 9, y: 8, z: 4}}"