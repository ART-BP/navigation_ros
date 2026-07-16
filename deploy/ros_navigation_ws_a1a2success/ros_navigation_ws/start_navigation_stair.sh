#!/bin/bash

gnome-terminal --window -e 'bash -c "source ~/ros_navigation_ws/install/setup.bash;roslaunch omni_navigation real_navigation_Gdog_stair.launch local_planner:=teb dog_name:=small_dog rviz:=true; exec bash"' \
--tab -e 'bash -c "sleep 3;python ~/ros_navigation_ws/script/twistConvert.py; exec bash"' \

#--tab -e 'bash -c "sleep 5;taskset -c 2 conda run -n unitree python ~/ros_navigation_ws/src/omni_robot/omni_teleop/script/set_initial_pose.py; exec bash"'
                                                       
