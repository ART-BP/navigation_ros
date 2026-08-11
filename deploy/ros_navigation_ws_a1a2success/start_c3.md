
#  启动slam，需要在起点附近
cd ~/slam_ws
./lidar*_zyy_C3.sh


#  启动导航
cd ~/ros_navigation_ws
./start*_C3.sh

#  目标点
cd ~/ros_navigation_ws/script/path_pub
python manual_waypoint_sender.py

