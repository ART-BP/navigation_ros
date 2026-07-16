#!/usr/bin/env python
# -*- coding: utf-8 -*-

import rospy
import tf2_ros


def get_pose(target_frame="map", source_frame="base_link", timeout=5.0):
    """
    从 /tf 中读取 target_frame -> source_frame 的最新变换，并返回当前位置信息。
    默认读取 map -> base_link。
    """
    if not rospy.core.is_initialized():
        rospy.init_node("init_pose_reader", anonymous=True)

    tf_buffer = tf2_ros.Buffer()
    tf2_ros.TransformListener(tf_buffer)

    rospy.sleep(0.2)

    try:
        transform = tf_buffer.lookup_transform(
            target_frame, source_frame, rospy.Time(0), rospy.Duration(timeout)
        )
    except (
        tf2_ros.LookupException,
        tf2_ros.ConnectivityException,
        tf2_ros.ExtrapolationException,
    ) as exc:
        rospy.logerr("读取 %s -> %s 失败: %s", target_frame, source_frame, str(exc))
        return None

    position = (
        transform.transform.translation.x,
        transform.transform.translation.y,
        transform.transform.translation.z,
    )
    orientation = (
        transform.transform.rotation.x,
        transform.transform.rotation.y,
        transform.transform.rotation.z,
        transform.transform.rotation.w,
    )

    result = {
        "target_frame": target_frame,
        "source_frame": source_frame,
        "position": position,
        "orientation": orientation,
        "stamp": transform.header.stamp.to_sec(),
    }

    rospy.loginfo(
        "当前位姿 %s -> %s: position(x=%.3f, y=%.3f, z=%.3f)",
        target_frame,
        source_frame,
        position[0],
        position[1],
        position[2],
    )
    return result


if __name__ == "__main__":
    target = rospy.get_param("~target_frame", "map")
    source = rospy.get_param("~source_frame", "odom")
    timeout = float(rospy.get_param("~timeout", 5.0))

    pose = get_pose(target_frame=target, source_frame=source, timeout=timeout)
    if pose is not None:
        print(pose)
