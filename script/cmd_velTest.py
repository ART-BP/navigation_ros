#!/usr/bin/env python3
import rospy
from geometry_msgs.msg import Twist


def publish_cmd(pub, rate, linear_x, angular_z, duration, label=""):
    cmd = Twist()
    cmd.linear.x = linear_x
    cmd.linear.y = 0.0
    cmd.linear.z = 0.0

    cmd.angular.x = 0.0
    cmd.angular.y = 0.0
    cmd.angular.z = angular_z

    count = int(duration * rate.sleep_dur.to_sec() ** -1)

    rospy.loginfo(
        f"{label}: linear.x={linear_x:.2f} m/s, "
        f"angular.z={angular_z:.2f} rad/s, duration={duration:.1f}s"
    )

    for _ in range(count):
        if rospy.is_shutdown():
            break
        pub.publish(cmd)
        rate.sleep()


def stop_robot(pub):
    stop = Twist()
    pub.publish(stop)


def main():
    rospy.init_node("cmd_vel_sequence_publisher")

    topic = rospy.get_param("~topic", "/cmd_vel")
    hz = rospy.get_param("~hz", 10.0)

    pub = rospy.Publisher(topic, Twist, queue_size=10)
    rate = rospy.Rate(hz)

    rospy.loginfo(f"Publishing cmd_vel sequence to {topic} at {hz} Hz")

    # 等待 publisher 建立连接
    rospy.sleep(0.5)

    sequence = [
        # linear_x, angular_z, duration, label
        (0.0,  0.0, 5.0, "Stop 5s"),
        (0.5,  0.0, 5.0, "Forward 0.5 m/s 5s"),
        (1.0,  0.0, 3.0, "Forward 1.0 m/s 3s"),
        (0.0,  0.0, 5.0, "Stop 5s"),

        (0.0,  0.5, 5.0, "Turn left 0.5 rad/s 5s"),
        (0.0,  1.0, 3.0, "Turn left 1.0 rad/s 3s"),
        (0.0, -0.5, 5.0, "Turn right 0.5 rad/s 5s"),
        (0.0, -1.0, 3.0, "Turn right 1.0 rad/s 3s"),
    ]

    try:
        for linear_x, angular_z, duration, label in sequence:
            publish_cmd(pub, rate, linear_x, angular_z, duration, label)

    finally:
        rospy.loginfo("Sequence finished. Publishing stop command.")
        for _ in range(10):
            stop_robot(pub)
            rate.sleep()


if __name__ == "__main__":
    main()