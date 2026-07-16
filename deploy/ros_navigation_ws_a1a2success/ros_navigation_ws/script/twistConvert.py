#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import rospy
from geometry_msgs.msg import Twist, TwistStamped


class TwistToTwistStamped:
    def __init__(self):
        # 订阅 move_base 输出的 cmd_vel
        self.input_topic = rospy.get_param("~input_topic", "/cmd_vel")

        # 发布机器人底盘需要的 TwistStamped
        self.output_topic = rospy.get_param("~output_topic", "/ground_robot/cmd_vel")

        # frame_id 一般填 base_link，也可以按你的底盘接口要求修改
        self.frame_id = rospy.get_param("~frame_id", "base_link")

        self.pub = rospy.Publisher(
            self.output_topic,
            TwistStamped,
            queue_size=10
        )

        self.sub = rospy.Subscriber(
            self.input_topic,
            Twist,
            self.cmd_vel_callback,
            queue_size=10
        )

        rospy.loginfo("Twist to TwistStamped converter started.")
        rospy.loginfo("Subscribe: %s", self.input_topic)
        rospy.loginfo("Publish:   %s", self.output_topic)
        rospy.loginfo("Frame ID:  %s", self.frame_id)

    def cmd_vel_callback(self, msg):
        out_msg = TwistStamped()

        out_msg.header.stamp = rospy.Time.now()
        out_msg.header.frame_id = self.frame_id

        out_msg.twist = msg

        self.pub.publish(out_msg)


if __name__ == "__main__":
    rospy.init_node("twist_to_twist_stamped")
    node = TwistToTwistStamped()
    rospy.spin()