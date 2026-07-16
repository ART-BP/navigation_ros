#!/usr/bin/env python3
import math

import actionlib
import rospy
from actionlib_msgs.msg import GoalStatus
from move_base_msgs.msg import MoveBaseAction, MoveBaseGoal


# x, y, yaw（角度）
WAYPOINTS = [
    (4.2, 0.866, -90.0),
    (3.099, -12.700, -90.0),
    (-1.784, -14.884, 170.0),
    (-6.057, -15.021, 170),
    (-11.088, -16.010, -105.0),
    (-9.408, -22.254, -71.94),
    (-5.937, -24.320, 85.0),
]


def make_goal(x, y, yaw_deg):
    yaw = math.radians(yaw_deg)
    goal = MoveBaseGoal()
    goal.target_pose.header.frame_id = "map"
    goal.target_pose.header.stamp = rospy.Time.now()
    goal.target_pose.pose.position.x = x
    goal.target_pose.pose.position.y = y
    goal.target_pose.pose.orientation.z = math.sin(yaw / 2.0)
    goal.target_pose.pose.orientation.w = math.cos(yaw / 2.0)
    return goal


def main():
    rospy.init_node("manual_waypoint_sender")
    client = actionlib.SimpleActionClient("/move_base", MoveBaseAction)

    rospy.loginfo("Waiting for /move_base...")
    client.wait_for_server()

    try:
        for index, (x, y, yaw_deg) in enumerate(WAYPOINTS, start=1):
            answer = input(
                "[{}/{}] x={:.6f}, y={:.6f}, yaw={:.6f} deg; "
                "press Enter to send, or q to quit: ".format(
                    index, len(WAYPOINTS), x, y, yaw_deg
                )
            ).strip().lower()
            if answer == "q":
                break

            client.send_goal(make_goal(x, y, yaw_deg))
            # client.wait_for_result()

            # state = client.get_state()
            # if state == GoalStatus.SUCCEEDED:
            rospy.loginfo("Goal %d reached", index)
            # else:
            #     rospy.logwarn("Goal %d finished with state %d", index, state)
    except (KeyboardInterrupt, rospy.ROSInterruptException):
        client.cancel_goal()


if __name__ == "__main__":
    main()
