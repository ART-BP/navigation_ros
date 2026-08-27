#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import math
import time

import rospy
import tf2_ros
from geometry_msgs.msg import Twist, TwistStamped
from std_msgs.msg import String


# 在这里填写实际路径点。中间点推荐只写 (x, y)，其角度不会参与判定；
# 最终点如需指定朝向，可写成 (x, y, yaw_deg)；没有朝向时可写
# (x, y) 或 (x, y, None)，到达位置后不会再调整角度。
WAYPOINTS = [
    # (1.0, 2.0),
    # (2.5, 2.0),
    # (4.0, 3.0, 90.0),  # 最终点及最终朝向
]

# 全部目标点成功到达后发送到 /speak 的内容，请在这里填写。
SUCCESS_MESSAGE = ""


def clamp(value, lower, upper):
    return max(lower, min(upper, value))


def normalize_angle(angle):
    return math.atan2(math.sin(angle), math.cos(angle))


def quaternion_to_yaw(quaternion):
    sin_yaw = 2.0 * (
        quaternion.w * quaternion.z + quaternion.x * quaternion.y
    )
    cos_yaw = 1.0 - 2.0 * (
        quaternion.y * quaternion.y + quaternion.z * quaternion.z
    )
    return math.atan2(sin_yaw, cos_yaw)


def parse_waypoint(waypoint, index):
    if len(waypoint) == 2:
        return float(waypoint[0]), float(waypoint[1]), None
    if len(waypoint) == 3:
        if waypoint[2] is None:
            return float(waypoint[0]), float(waypoint[1]), None
        return (
            float(waypoint[0]),
            float(waypoint[1]),
            math.radians(float(waypoint[2])),
        )
    raise ValueError(
        "Waypoint {} must be (x, y), (x, y, None), or "
        "(x, y, yaw_deg)".format(index + 1)
    )


class TfWaypointFollower:
    def __init__(self):
        self.map_frame = rospy.get_param("~map_frame", "map")
        self.base_frame = rospy.get_param("~base_frame", "base_link")
        self.cmd_vel_topic = rospy.get_param(
            "~cmd_vel_topic", "/ground_robot/cmd_vel"
        )
        self.cmd_frame_id = rospy.get_param("~cmd_frame_id", self.base_frame)
        self.speak_topic = rospy.get_param("~speak_topic", "/speak")
        self.success_message = rospy.get_param(
            "~success_message", SUCCESS_MESSAGE
        )

        # 高频闭环控制参数。
        self.control_rate = float(rospy.get_param("~control_rate", 50.0))
        self.max_linear_speed = float(rospy.get_param("~max_linear_speed", 0.55))
        self.min_linear_speed = float(rospy.get_param("~min_linear_speed", 0.08))
        self.max_angular_speed = float(rospy.get_param("~max_angular_speed", 1.0))
        self.min_angular_speed = float(rospy.get_param("~min_angular_speed", 0.25))
        self.linear_kp = float(rospy.get_param("~linear_kp", 0.8))
        self.angular_kp = float(rospy.get_param("~angular_kp", 2.5))

        # 路径较窄：中间点必须靠近后才能切换，但不检查中间点 yaw。
        # 最终点仍同时检查位置和朝向，且使用稍宽松的容差。
        self.intermediate_tolerance = float(
            rospy.get_param("~intermediate_tolerance", 0.15)
        )
        self.position_tolerance = float(
            rospy.get_param("~position_tolerance", 0.30)
        )
        self.yaw_tolerance = math.radians(
            float(rospy.get_param("~yaw_tolerance_deg", 10.0))
        )
        self.min_turn_error = math.radians(
            float(rospy.get_param("~min_turn_error_deg", 3.0))
        )
        # 每次进入新路段时，先原地转到该误差以内再起步。
        self.start_moving_angle = math.radians(
            float(rospy.get_param("~start_moving_angle_deg", 12.0))
        )
        # 行驶中偏差超过该值时，重新停车对准，防止偏出狭窄路径。
        self.turn_in_place_angle = math.radians(
            float(rospy.get_param("~turn_in_place_angle_deg", 30.0))
        )
        self.slowdown_distance = float(
            rospy.get_param("~slowdown_distance", 1.0)
        )

        # TF 超过该时间未更新时，不再计算运动指令。
        self.tf_lookup_timeout = float(rospy.get_param("~tf_lookup_timeout", 0.05))
        self.max_tf_age = float(rospy.get_param("~max_tf_age", 0.5))
        self.waypoint_timeout = float(rospy.get_param("~waypoint_timeout", 180.0))

        # 标准 ROS 坐标中，angular.z > 0 表示左转。如果底盘方向相反，
        # 启动时设置 _angular_sign:=-1.0。
        self.angular_sign = float(rospy.get_param("~angular_sign", 1.0))

        self._validate_parameters()

        self.cmd_pub = rospy.Publisher(
            self.cmd_vel_topic, TwistStamped, queue_size=1
        )
        self.speak_pub = rospy.Publisher(
            self.speak_topic, String, queue_size=1, latch=True
        )
        self.tf_buffer = tf2_ros.Buffer(cache_time=rospy.Duration(5.0))
        self.tf_listener = tf2_ros.TransformListener(self.tf_buffer)
        self.rate = rospy.Rate(self.control_rate)

        rospy.on_shutdown(self.stop_once)
        rospy.loginfo(
            "TF waypoint follower: %s -> %s, cmd=%s, rate=%.1f Hz",
            self.map_frame,
            self.base_frame,
            self.cmd_vel_topic,
            self.control_rate,
        )

    def _validate_parameters(self):
        if not isinstance(self.success_message, str):
            raise ValueError("~success_message must be a string")

        positive_values = {
            "control_rate": self.control_rate,
            "max_linear_speed": self.max_linear_speed,
            "max_angular_speed": self.max_angular_speed,
            "linear_kp": self.linear_kp,
            "angular_kp": self.angular_kp,
            "intermediate_tolerance": self.intermediate_tolerance,
            "position_tolerance": self.position_tolerance,
            "yaw_tolerance": self.yaw_tolerance,
            "start_moving_angle": self.start_moving_angle,
            "turn_in_place_angle": self.turn_in_place_angle,
            "slowdown_distance": self.slowdown_distance,
            "tf_lookup_timeout": self.tf_lookup_timeout,
            "max_tf_age": self.max_tf_age,
        }
        for name, value in positive_values.items():
            if not math.isfinite(value) or value <= 0.0:
                raise ValueError("~{} must be finite and greater than zero".format(name))

        nonnegative_values = {
            "min_linear_speed": self.min_linear_speed,
            "min_angular_speed": self.min_angular_speed,
            "min_turn_error": self.min_turn_error,
            "waypoint_timeout": self.waypoint_timeout,
        }
        for name, value in nonnegative_values.items():
            if not math.isfinite(value) or value < 0.0:
                raise ValueError("~{} must be finite and nonnegative".format(name))

        if self.min_linear_speed > self.max_linear_speed:
            raise ValueError("~min_linear_speed cannot exceed ~max_linear_speed")
        if self.min_angular_speed > self.max_angular_speed:
            raise ValueError("~min_angular_speed cannot exceed ~max_angular_speed")
        if self.start_moving_angle >= self.turn_in_place_angle:
            raise ValueError(
                "~start_moving_angle_deg must be smaller than "
                "~turn_in_place_angle_deg"
            )
        if self.angular_sign not in (-1.0, 1.0):
            raise ValueError("~angular_sign must be 1.0 or -1.0")

    def get_robot_pose(self):
        transform = self.tf_buffer.lookup_transform(
            self.map_frame,
            self.base_frame,
            rospy.Time(0),
            rospy.Duration(self.tf_lookup_timeout),
        )

        stamp = transform.header.stamp
        if stamp != rospy.Time(0):
            tf_age = (rospy.Time.now() - stamp).to_sec()
            if tf_age < 0.0 or tf_age > self.max_tf_age:
                raise RuntimeError(
                    "TF is stale: age={:.3f}s, allowed={:.3f}s".format(
                        tf_age, self.max_tf_age
                    )
                )

        translation = transform.transform.translation
        yaw = quaternion_to_yaw(transform.transform.rotation)
        return translation.x, translation.y, yaw

    def angular_command(self, angle_error):
        command = clamp(
            self.angular_kp * angle_error,
            -self.max_angular_speed,
            self.max_angular_speed,
        )
        if (
            abs(angle_error) >= self.min_turn_error
            and abs(command) < self.min_angular_speed
        ):
            command = math.copysign(self.min_angular_speed, angle_error)
        return self.angular_sign * command

    def driving_command(self, distance, heading_error):
        cmd = Twist()
        cmd.angular.z = self.angular_command(heading_error)

        if abs(heading_error) >= self.turn_in_place_angle:
            return cmd

        linear_speed = min(
            self.max_linear_speed,
            self.linear_kp * distance,
            self.max_linear_speed * distance / self.slowdown_distance,
        )
        # 小角度行驶修正时平滑降低线速度。
        heading_scale = max(0.0, math.cos(heading_error))
        linear_speed *= heading_scale * heading_scale
        if distance > self.position_tolerance:
            linear_speed = max(self.min_linear_speed, linear_speed)
        cmd.linear.x = linear_speed
        return cmd

    def publish_twist(self, twist):
        message = TwistStamped()
        message.header.stamp = rospy.Time.now()
        message.header.frame_id = self.cmd_frame_id
        message.twist = twist
        self.cmd_pub.publish(message)

    def stop_once(self):
        if hasattr(self, "cmd_pub"):
            self.publish_twist(Twist())

    def stop_robot(self, cycles=5):
        stop_cmd = Twist()
        for _ in range(cycles):
            if rospy.is_shutdown():
                break
            self.publish_twist(stop_cmd)
            self.rate.sleep()

    def follow_waypoint(self, waypoint, index, total, is_final):
        target_x, target_y, target_yaw = parse_waypoint(waypoint, index)
        started_at = time.monotonic()
        active_position_tolerance = (
            self.position_tolerance if is_final else self.intermediate_tolerance
        )
        align_yaw = target_yaw is not None and is_final
        position_reached = False
        heading_aligned = False

        rospy.loginfo(
            "Following waypoint %d/%d: x=%.3f, y=%.3f, yaw=%s",
            index + 1,
            total,
            target_x,
            target_y,
            (
                "none"
                if target_yaw is None
                else "{:.1f} deg".format(math.degrees(target_yaw))
                if is_final
                else "ignored for intermediate waypoint"
            ),
        )

        while not rospy.is_shutdown():
            if (
                self.waypoint_timeout > 0.0
                and time.monotonic() - started_at > self.waypoint_timeout
            ):
                rospy.logerr("Waypoint %d timed out", index + 1)
                self.stop_robot()
                return False

            try:
                robot_x, robot_y, robot_yaw = self.get_robot_pose()
            except (tf2_ros.TransformException, RuntimeError) as exc:
                self.stop_once()
                rospy.logwarn_throttle(1.0, "Cannot use TF; robot stopped: {}".format(exc))
                self.rate.sleep()
                continue

            dx = target_x - robot_x
            dy = target_y - robot_y
            distance = math.hypot(dx, dy)

            if distance <= active_position_tolerance:
                position_reached = True

            if position_reached:
                if not align_yaw:
                    if is_final:
                        self.stop_once()
                    return True

                yaw_error = normalize_angle(target_yaw - robot_yaw)
                if abs(yaw_error) <= self.yaw_tolerance:
                    if is_final:
                        self.stop_once()
                    return True

                cmd = Twist()
                cmd.angular.z = self.angular_command(yaw_error)
                self.publish_twist(cmd)
                rospy.loginfo_throttle(
                    1.0,
                    "Waypoint {}/{} position reached; aligning yaw, error={:.1f} deg".format(
                        index + 1, total, math.degrees(yaw_error)
                    ),
                )
            else:
                target_heading = math.atan2(dy, dx)
                heading_error = normalize_angle(target_heading - robot_yaw)

                # 新路段必须先对准；行驶过程中偏差过大时也重新对准。
                if heading_aligned and abs(heading_error) >= self.turn_in_place_angle:
                    heading_aligned = False

                if not heading_aligned:
                    if abs(heading_error) <= self.start_moving_angle:
                        heading_aligned = True
                    else:
                        cmd = Twist()
                        cmd.angular.z = self.angular_command(heading_error)
                        self.publish_twist(cmd)
                        rospy.loginfo_throttle(
                            1.0,
                            "Waypoint {}/{} aligning before driving, "
                            "heading_error={:.1f}deg cmd(w={:.3f})".format(
                                index + 1,
                                total,
                                math.degrees(heading_error),
                                cmd.angular.z,
                            ),
                        )
                        self.rate.sleep()
                        continue

                cmd = self.driving_command(distance, heading_error)
                self.publish_twist(cmd)
                rospy.loginfo_throttle(
                    1.0,
                    "Waypoint {}/{} distance={:.3f}m heading_error={:.1f}deg "
                    "cmd(v={:.3f}, w={:.3f})".format(
                        index + 1,
                        total,
                        distance,
                        math.degrees(heading_error),
                        cmd.linear.x,
                        cmd.angular.z,
                    ),
                )

            self.rate.sleep()

        return False

    def run(self):
        if not WAYPOINTS:
            rospy.logwarn("No waypoints configured")
            self.stop_robot()
            return False

        try:
            for index, waypoint in enumerate(WAYPOINTS):
                is_final = index == len(WAYPOINTS) - 1
                if not self.follow_waypoint(
                    waypoint, index, len(WAYPOINTS), is_final
                ):
                    return False
                if is_final:
                    rospy.loginfo(
                        "Final waypoint %d/%d reached", index + 1, len(WAYPOINTS)
                    )
                else:
                    rospy.loginfo(
                        "Waypoint %d/%d passed; continuing without stopping",
                        index + 1,
                        len(WAYPOINTS),
                    )

            rospy.loginfo("All %d waypoints reached", len(WAYPOINTS))
            self.speak_pub.publish(String(data=self.success_message))
            rospy.loginfo("Published arrival feedback to %s", self.speak_topic)
            return True
        finally:
            self.stop_robot(cycles=10)


def main():
    rospy.init_node("tf_waypoint_follower")
    follower = TfWaypointFollower()
    follower.run()


if __name__ == "__main__":
    main()
