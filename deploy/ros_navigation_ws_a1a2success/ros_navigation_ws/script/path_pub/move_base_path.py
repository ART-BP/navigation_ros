
#!/usr/bin/env python
# -*- coding: utf-8 -*-

import math
import rospy
import actionlib
from move_base_msgs.msg import MoveBaseAction, MoveBaseGoal

class PathNavigator:
    def __init__(self):
        rospy.init_node('path_navigator', anonymous=True)
        
        self.waypoints = [
            (-35.425357, 59.911787, -74.086572),
            (-33.164115, 49.838446, -80.677715),
            (-32.958688, 38.728581, -96.598264),
            (-33.420984, 29.678354, -74.240537),
            (-31.411086, 19.702001, -73.980941),
            (-29.079681, 9.540616, -77.444076),
            (-25.751863, -0.217771, -25.198271),
            (-16.432366, -1.489931, 10.589998),
            (-6.046448, 0.872025, 12.911895),
            (4.022985, 2.678638, 8.817706),
            (14.993074, 3.791087, 0.022461),
            (25.386206, 2.890671, 34.542417),
            (37.861624, 10.911145, 63.674854),
            (39.372735, 20.796521, 87.521686),
            (39.689409, 31.235665, 82.738039),
            (41.811802, 41.240727, 76.913705),
            (44.417112, 51.240483, 76.076453),
            (46.337165, 61.263785, 82.733060),
            (47.252835, 71.281662, 90.148425),
            (48.141499, 81.800032, 81.821294),
            (50.021617, 92.046050, 83.630225),
            (49.362647, 102.657667, 101.507269),
            (48.214993, 112.666614, 86.204300),
            (46.400000, 120.000000, 89.426910),
            (46.123482, 128.547640, 134.426910),
            (41.554874, 131.188375, -172.638757),
            (30.495931, 130.290693, -179.663707),
            (20.060584, 129.617804, 177.360155),
            (9.739099, 129.657840, -158.224018),
            (3.475476, 127.297128, -122.236947),
            (2.562862, 126.252202, -156.892461),
            (-7.679565, 125.568471, -176.415247),
            (-18.108431, 124.545495, 176.888559),
            (-28.096619, 123.538557, -168.119826),
            (-37.404815, 119.829267, -142.376332),
            (-43.279099, 111.486254, -115.904880),
            (-47.206456, 102.016323, -109.221511),
            (-47.029900, 92.986246, -78.786368),
            (-43.713811, 82.526297, -71.592631),
            (-38.968017, 72.642244, -73.782076),
        ]
        self.current_waypoint_idx = 0
        self.total_waypoints = len(self.waypoints)
        
        self.client = actionlib.SimpleActionClient('move_base', MoveBaseAction)
        self.client.wait_for_server(rospy.Duration(5.0))
        
        rospy.loginfo("路径导航器已初始化，共有 %d 个目标点", self.total_waypoints)
    
    def send_goal(self, x, y, yaw_deg):
        goal = MoveBaseGoal()
        goal.target_pose.header.frame_id = "map"
        goal.target_pose.header.stamp = rospy.Time.now()
        goal.target_pose.pose.position.x = x
        goal.target_pose.pose.position.y = y
        goal.target_pose.pose.position.z = 0.0
        
        yaw_rad = math.radians(yaw_deg)
        goal.target_pose.pose.orientation.x = 0.0
        goal.target_pose.pose.orientation.y = 0.0
        goal.target_pose.pose.orientation.z = math.sin(yaw_rad / 2.0)
        goal.target_pose.pose.orientation.w = math.cos(yaw_rad / 2.0)
        
        rospy.loginfo("发送目标点 %d/%d: (%.2f, %.2f, %.2f deg)", 
                     self.current_waypoint_idx + 1, 
                     self.total_waypoints, 
                     x, y, yaw_deg)
        
        # 不要传 done_cb 回调！改用 wait_for_result 轮询
        self.client.send_goal(goal)
    
    def navigate_waypoints(self):
        """顺序导航所有目标点 - 使用轮询方式避免状态机冲突"""
        while self.current_waypoint_idx < self.total_waypoints and not rospy.is_shutdown():
            wp = self.waypoints[self.current_waypoint_idx]
            self.send_goal(wp[0], wp[1], wp[2])
            
            # 阻塞等待当前目标完成（带超时）
            finished = self.client.wait_for_result(rospy.Duration(60.0))
            
            if not finished:
                rospy.logwarn("目标点 %d 超时，取消并尝试下一个", self.current_waypoint_idx + 1)
                self.client.cancel_goal()
                rospy.sleep(1.0)
            else:
                state = self.client.get_state()
                if state == actionlib.GoalStatus.SUCCEEDED:
                    rospy.loginfo("目标点 %d 已到达！", self.current_waypoint_idx + 1)
                else:
                    rospy.logwarn("目标点 %d 失败，状态: %d", self.current_waypoint_idx + 1, state)
            
            self.current_waypoint_idx += 1
        
        rospy.loginfo("所有目标点处理完毕！")

    def start_navigation(self):
        if self.total_waypoints > 0:
            self.navigate_waypoints()
        else:
            rospy.logwarn("没有目标点可导航！")

if __name__ == '__main__':
    try:
        navigator = PathNavigator()
        navigator.start_navigation()
    except rospy.ROSInterruptException:
        rospy.loginfo("导航程序已停止")
