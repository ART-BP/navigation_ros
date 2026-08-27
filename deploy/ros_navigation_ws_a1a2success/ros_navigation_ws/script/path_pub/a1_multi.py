#!/usr/bin/env python3
import math
import actionlib
import rospy
from actionlib_msgs.msg import GoalStatus
from move_base_msgs.msg import MoveBaseAction, MoveBaseGoal
from std_msgs.msg import String


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
    pub_ = rospy.Publisher("/speak", String, latch=True, queue_size=1)

    rospy.loginfo("Waiting for /move_base...")
    client.wait_for_server()

    try:
        all_succeeded = bool(WAYPOINTS)

        for index, (x, y, yaw_deg) in enumerate(WAYPOINTS, start=1):
            client.send_goal(make_goal(x, y, yaw_deg))

            finished = client.wait_for_result(rospy.Duration(10.0))
            if not finished:
                client.cancel_goal()
                all_succeeded = False
                rospy.logwarn("Goal %d timed out", index)
                continue

            state = client.get_state()
            if state == GoalStatus.SUCCEEDED:
                rospy.loginfo("Goal %d reached", index)
            else:
                all_succeeded = False
                rospy.logwarn("Goal %d finished with state %d", index, state)

        if all_succeeded:
            pub_.publish("{\"mode\":\"local\",\"group\":\"Navigation\",\"text\":\"已到达目的地\"}")
        elif not WAYPOINTS:
            rospy.logwarn("No waypoints configured")
        else:
            rospy.logwarn("Not all goals succeeded; feedback was not published")
    except (KeyboardInterrupt, rospy.ROSInterruptException):
        client.cancel_goal()


if __name__ == "__main__":
    main()
