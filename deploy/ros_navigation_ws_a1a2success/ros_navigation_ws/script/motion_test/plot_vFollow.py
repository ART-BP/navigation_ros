#!/usr/bin/env python3
import argparse
import rosbag
import matplotlib.pyplot as plt
from scipy.ndimage import median_filter


def get_time_sec(t, t0):
    return t.to_sec() - t0


def extract_velocity(msg):
    """
    支持常见速度消息：
    1. geometry_msgs/Twist
    2. geometry_msgs/TwistStamped
    3. nav_msgs/Odometry
    """
    # geometry_msgs/Twist
    if hasattr(msg, "linear") and hasattr(msg, "angular"):
        return msg.linear.x, msg.linear.y, msg.angular.z

    # geometry_msgs/TwistStamped
    if hasattr(msg, "twist"):
        twist = msg.twist

        # TwistStamped: msg.twist.linear.x
        if hasattr(twist, "linear") and hasattr(twist, "angular"):
            return twist.linear.x, twist.linear.y, twist.angular.z

        # Odometry: msg.twist.twist.linear.x
        if hasattr(twist, "twist"):
            return (
                twist.twist.linear.x,
                twist.twist.linear.y,
                twist.twist.angular.z,
            )

    raise TypeError(f"Unsupported message type: {type(msg)}")


def median_filter_velocity(values, window_size):
    if not values:
        return values

    return median_filter(values, size=window_size, mode="nearest")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("bag_path", default="2026-06-15-10-47-50.bag")
    parser.add_argument("--cmd_topic", default="/cmd_vel", help="cmd_vel topic name")
    parser.add_argument("--loc_topic", default="/loc_base", help="loc_base velocity topic name")
    parser.add_argument(
        "--median_window",
        type=int,
        default=101,
        help="Median filter window size for loc_base data (positive odd integer)",
    )
    parser.add_argument("--save", default="", help="Save figure path, e.g. result.png")
    args = parser.parse_args()

    if args.median_window <= 0 or args.median_window % 2 == 0:
        parser.error("--median_window must be a positive odd integer")

    cmd_t = []
    cmd_vx = []
    cmd_vy = []
    cmd_wz = []

    loc_t = []
    loc_vx = []
    loc_vy = []
    loc_wz = []

    with rosbag.Bag(args.bag_path, "r") as bag:
        start_time = bag.get_start_time()

        topics = [args.cmd_topic, args.loc_topic]

        for topic, msg, t in bag.read_messages(topics=topics):
            time_sec = get_time_sec(t, start_time)

            try:
                vx, vy, wz = extract_velocity(msg)
            except TypeError as e:
                print(f"[WARN] {topic}: {e}")
                continue

            if topic == args.cmd_topic:
                cmd_t.append(time_sec)
                cmd_vx.append(vx)
                cmd_vy.append(vy)
                cmd_wz.append(wz)

            elif topic == args.loc_topic:
                loc_t.append(time_sec)
                loc_vx.append(vx)
                loc_vy.append(vy)
                loc_wz.append(wz)

    print(f"[INFO] {args.cmd_topic}: {len(cmd_t)} messages")
    print(f"[INFO] {args.loc_topic}: {len(loc_t)} messages")

    if len(cmd_t) == 0:
        print(f"[ERROR] No data found on {args.cmd_topic}")

    if len(loc_t) == 0:
        print(f"[ERROR] No data found on {args.loc_topic}")

    loc_vx = median_filter_velocity(loc_vx, args.median_window)
    loc_vy = median_filter_velocity(loc_vy, args.median_window)
    loc_wz = median_filter_velocity(loc_wz, args.median_window)
    print(f"[INFO] Applied median filter to {args.loc_topic}: window={args.median_window}")

    plt.figure(figsize=(12, 8))

    # 线速度 x
    plt.subplot(3, 1, 1)
    plt.plot(cmd_t, cmd_vx, label="cmd_vel linear.x")
    plt.plot(loc_t, loc_vx, label="loc_base linear.x (median filtered)")
    plt.ylabel("vx / m/s")
    plt.grid(True)
    plt.legend()

    # 线速度 y
    plt.subplot(3, 1, 2)
    plt.plot(cmd_t, cmd_vy, label="cmd_vel linear.y")
    plt.plot(loc_t, loc_vy, label="loc_base linear.y (median filtered)")
    plt.ylabel("vy / m/s")
    plt.grid(True)
    plt.legend()

    # 角速度 z
    plt.subplot(3, 1, 3)
    plt.plot(cmd_t, cmd_wz, label="cmd_vel angular.z")
    plt.plot(loc_t, loc_wz, label="loc_base angular.z (median filtered)")
    plt.xlabel("time / s")
    plt.ylabel("wz / rad/s")
    plt.grid(True)
    plt.legend()

    plt.suptitle("cmd_vel vs loc_base velocity")
    plt.tight_layout()

    if args.save:
        plt.savefig(args.save, dpi=300)
        print(f"[INFO] Figure saved to: {args.save}")

    plt.show()


if __name__ == "__main__":
    main()
