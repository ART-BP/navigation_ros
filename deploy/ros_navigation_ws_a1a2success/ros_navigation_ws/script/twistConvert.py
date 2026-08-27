#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import copy
import threading

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

        # 输入约为 5 Hz（周期 0.2 秒）。超过该时间未收到新数据后，
        # 停止重发旧速度。该参数应略大于正常输入周期，以容忍少量抖动。
        self.output_rate = float(rospy.get_param("~output_rate", 50.0))
        self.input_timeout = float(rospy.get_param("~input_timeout", 0.25))
        if self.output_rate <= 0.0:
            raise ValueError("~output_rate must be greater than zero")
        if self.input_timeout <= 0.0:
            raise ValueError("~input_timeout must be greater than zero")

        self.lock = threading.Lock()
        self.latest_cmd = None
        self.latest_cmd_time = None
        self.input_timed_out = False

        self.pub = rospy.Publisher(
            self.output_topic,
            TwistStamped,
            queue_size=1
        )

        self.sub = rospy.Subscriber(
            self.input_topic,
            Twist,
            self.cmd_vel_callback,
            queue_size=1
        )

        self.publish_timer = rospy.Timer(
            rospy.Duration.from_sec(1.0 / self.output_rate),
            self.publish_latest_cmd
        )


    def cmd_vel_callback(self, msg):
        # 保存一份完整快照，确保发布期间的数据不会被修改。
        with self.lock:
            self.latest_cmd = copy.deepcopy(msg)
            self.latest_cmd_time = rospy.Time.now()
            was_timed_out = self.input_timed_out
            self.input_timed_out = False

        if was_timed_out:
            rospy.loginfo("Input resumed; restarting output")

    def publish_latest_cmd(self, _event):
        now = rospy.Time.now()

        with self.lock:
            if self.latest_cmd is None or self.latest_cmd_time is None:
                return

            age = (now - self.latest_cmd_time).to_sec()
            if age > self.input_timeout:
                if not self.input_timed_out:
                    rospy.logwarn(
                        "No %s data for %.3f s; stopping %s output",
                        self.input_topic,
                        age,
                        self.output_topic
                    )
                self.input_timed_out = True
                self.latest_cmd = None
                self.latest_cmd_time = None
                return

            cmd = copy.deepcopy(self.latest_cmd)

        out_msg = TwistStamped()
        out_msg.header.stamp = now
        out_msg.header.frame_id = self.frame_id
        out_msg.twist = cmd
        self.pub.publish(out_msg)


if __name__ == "__main__":
    rospy.init_node("twist_to_twist_stamped")
    node = TwistToTwistStamped()
    rospy.spin()
