#!/usr/bin/env python3
"""Publish one configurable dynamic_obstacles/TrackedObject for testing.

Example:
  python3 tracked_objects_test_publisher.py \
      --start-x 10.0 --start-y 2.0 \
      --end-x 0.0 --end-y 2.0 \
      --speed 1.0 --end-behavior disappear
"""

import argparse
import math
import time

import rospy

from dynamic_obstacles.msg import TrackedObject, TrackedObjectArray
from geometry_msgs.msg import Point
from visualization_msgs.msg import Marker, MarkerArray


def finite_float(value):
    parsed = float(value)
    if not math.isfinite(parsed):
        raise argparse.ArgumentTypeError("value must be finite")
    return parsed


def nonnegative_float(value):
    parsed = finite_float(value)
    if parsed < 0.0:
        raise argparse.ArgumentTypeError("value must be non-negative")
    return parsed


def positive_float(value):
    parsed = finite_float(value)
    if parsed <= 0.0:
        raise argparse.ArgumentTypeError("value must be positive")
    return parsed


def byte_value(value):
    parsed = int(value)
    if parsed < 0 or parsed > 255:
        raise argparse.ArgumentTypeError("value must be in [0, 255]")
    return parsed


def nonnegative_int(value):
    parsed = int(value)
    if parsed < 0:
        raise argparse.ArgumentTypeError("value must be non-negative")
    return parsed


def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Publish one tracked object moving from start to end in the map frame."
    )
    parser.add_argument("--start-x", type=finite_float, default=219.201626,
                        help="Start X in map coordinates (default: -3.93)")
    parser.add_argument("--start-y", type=finite_float, default=-141.087202,
                        help="Start Y in map coordinates (default: 5.47)")
    parser.add_argument("--end-x", type=finite_float, default=183.040085,
                        help="End X in map coordinates (default: -4.06)")
    parser.add_argument("--end-y", type=finite_float, default=-57.78853,
                        help="End Y in map coordinates (default: -6.45)")
    parser.add_argument("--speed", type=nonnegative_float, default=1.0,
                        help="Velocity magnitude in m/s (default: 1.0)")
    parser.add_argument("--rate", type=positive_float, default=5.0,
                        help="Publishing frequency in Hz (default: 5)")
    parser.add_argument("--topic", default="/tracked_objects")
    parser.add_argument("--marker-topic", default="/tracked_objects/markers",
                        help="RViz MarkerArray topic")
    parser.add_argument("--frame-id", default="map")
    parser.add_argument("--label", type=byte_value, default=10,
                        help="ObjectClassification label (default: 10/CAR)")
    parser.add_argument("--object-id", type=nonnegative_int, default=1)
    parser.add_argument("--z", type=finite_float, default=0.0)
    parser.add_argument("--length", type=positive_float, default=1.0)
    parser.add_argument("--width", type=positive_float, default=0.6)
    parser.add_argument("--height", type=positive_float, default=1.0)
    parser.add_argument(
        "--end-behavior",
        choices=("stop", "disappear", "loop"),
        default="stop",
        help="At the endpoint: remain stationary, publish an empty array, or restart",
    )
    return parser.parse_args(rospy.myargv()[1:])


def uuid_bytes(object_id):
    result = [0] * 16
    value = object_id
    for index in range(16):
        result[15 - index] = value & 0xFF
        value >>= 8
    if value:
        raise ValueError("object-id must fit in 128 bits")
    return result


class TrackedObjectPublisher:
    def __init__(self, args):
        self.args = args
        self.dx = args.end_x - args.start_x
        self.dy = args.end_y - args.start_y
        self.path_length = math.hypot(self.dx, self.dy)
        if self.path_length <= 1e-9:
            raise ValueError("start and end points must be different")

        self.unit_x = self.dx / self.path_length
        self.unit_y = self.dy / self.path_length
        self.velocity_x = args.speed * self.unit_x
        self.velocity_y = args.speed * self.unit_y
        self.yaw = math.atan2(self.dy, self.dx)
        self.object_uuid = uuid_bytes(args.object_id)
        self.publisher = rospy.Publisher(
            args.topic, TrackedObjectArray, queue_size=1
        )
        self.marker_publisher = rospy.Publisher(
            args.marker_topic, MarkerArray, queue_size=1
        )
        self.marker_lifetime = rospy.Duration(max(0.5, 2.5 / args.rate))
        self.start_time = self.wait_for_valid_time()
        self.last_time = self.start_time

        duration_text = "infinite" if args.speed <= 1e-9 else \
            "{:.3f} s".format(self.path_length / args.speed)
        rospy.loginfo(
            "tracked_objects test publisher: start=(%.3f, %.3f), end=(%.3f, %.3f), "
            "speed=%.3f m/s, velocity=(%.3f, %.3f), travel_time=%s, behavior=%s",
            args.start_x,
            args.start_y,
            args.end_x,
            args.end_y,
            args.speed,
            self.velocity_x,
            self.velocity_y,
            duration_text,
            args.end_behavior,
        )
        rospy.loginfo("RViz MarkerArray topic: %s", args.marker_topic)

    @staticmethod
    def wait_for_valid_time():
        while not rospy.is_shutdown():
            now = rospy.Time.now()
            if not now.is_zero():
                return now
            time.sleep(0.01)
        raise RuntimeError("ROS shut down before a valid clock was available")

    def route_state(self, elapsed):
        travelled = self.args.speed * elapsed
        reached_end = travelled >= self.path_length

        if self.args.end_behavior == "loop" and self.args.speed > 1e-9:
            route_distance = travelled % self.path_length
            moving = True
        else:
            route_distance = min(travelled, self.path_length)
            moving = self.args.speed > 1e-9 and not reached_end

        x = self.args.start_x + route_distance * self.unit_x
        y = self.args.start_y + route_distance * self.unit_y
        return x, y, moving, reached_end

    def build_object(self, x, y, moving, elapsed):
        tracked = TrackedObject()
        tracked.object_id.uuid = self.object_uuid
        tracked.classification.label = self.args.label
        tracked.classification.probability = 1.0
        tracked.track_state = TrackedObject.TRACK_CONFIRMED
        tracked.tracking_time = elapsed
        tracked.time_since_update = 0.0

        tracked.pose.pose.position.x = x
        tracked.pose.pose.position.y = y
        tracked.pose.pose.position.z = self.args.z
        tracked.pose.pose.orientation.z = math.sin(0.5 * self.yaw)
        tracked.pose.pose.orientation.w = math.cos(0.5 * self.yaw)
        tracked.has_pose_covariance = False

        tracked.has_linear_velocity = True
        tracked.has_angular_velocity = False
        tracked.has_linear_covariance = False
        tracked.has_angular_covariance = False
        if moving:
            tracked.twist.twist.linear.x = self.velocity_x
            tracked.twist.twist.linear.y = self.velocity_y
            tracked.motion_state = TrackedObject.MOTION_MOVING
        else:
            tracked.twist.twist.linear.x = 0.0
            tracked.twist.twist.linear.y = 0.0
            tracked.motion_state = TrackedObject.MOTION_STATIONARY

        tracked.has_acceleration = False
        tracked.has_acceleration_covariance = False
        tracked.orientation_availability = TrackedObject.ORIENTATION_AVAILABLE
        tracked.shape_type = TrackedObject.SHAPE_BOUNDING_BOX
        tracked.dimensions.x = self.args.length
        tracked.dimensions.y = self.args.width
        tracked.dimensions.z = self.args.height
        return tracked

    def marker(self, header, marker_id, marker_type, action=Marker.ADD):
        result = Marker()
        result.header = header
        result.ns = "tracked_objects_test"
        result.id = marker_id
        result.type = marker_type
        result.action = action
        result.pose.orientation.w = 1.0
        return result

    def route_marker(self, header):
        route = self.marker(header, 0, Marker.LINE_STRIP)
        route.scale.x = 0.06
        route.color.r = 0.1
        route.color.g = 0.9
        route.color.b = 0.3
        route.color.a = 0.9
        route.points = [
            Point(self.args.start_x, self.args.start_y, self.args.z + 0.03),
            Point(self.args.end_x, self.args.end_y, self.args.z + 0.03),
        ]
        return route

    def delete_dynamic_markers(self, header):
        return [
            self.marker(header, 1, Marker.CUBE, Marker.DELETE),
            self.marker(header, 2, Marker.ARROW, Marker.DELETE),
            self.marker(header, 3, Marker.TEXT_VIEW_FACING, Marker.DELETE),
        ]

    def object_markers(self, header, tracked):
        box = self.marker(header, 1, Marker.CUBE)
        box.pose.position.x = tracked.pose.pose.position.x
        box.pose.position.y = tracked.pose.pose.position.y
        box.pose.position.z = tracked.pose.pose.position.z + 0.5 * tracked.dimensions.z
        box.pose.orientation = tracked.pose.pose.orientation
        box.scale = tracked.dimensions
        box.color.r = 1.0
        box.color.g = 0.25
        box.color.b = 0.05
        box.color.a = 0.55
        box.lifetime = self.marker_lifetime

        speed_x = tracked.twist.twist.linear.x
        speed_y = tracked.twist.twist.linear.y
        speed = math.hypot(speed_x, speed_y)
        if speed > 1e-9:
            arrow = self.marker(header, 2, Marker.ARROW)
            arrow.points = [
                Point(
                    tracked.pose.pose.position.x,
                    tracked.pose.pose.position.y,
                    tracked.pose.pose.position.z + 0.5 * tracked.dimensions.z,
                ),
                Point(
                    tracked.pose.pose.position.x + 1.5 * speed_x,
                    tracked.pose.pose.position.y + 1.5 * speed_y,
                    tracked.pose.pose.position.z + 0.5 * tracked.dimensions.z,
                ),
            ]
            arrow.scale.x = 0.08
            arrow.scale.y = 0.16
            arrow.scale.z = 0.22
            arrow.color.r = 0.1
            arrow.color.g = 0.8
            arrow.color.b = 1.0
            arrow.color.a = 1.0
            arrow.lifetime = self.marker_lifetime
        else:
            arrow = self.marker(header, 2, Marker.ARROW, Marker.DELETE)

        text_marker = self.marker(header, 3, Marker.TEXT_VIEW_FACING)
        text_marker.pose.position.x = tracked.pose.pose.position.x
        text_marker.pose.position.y = tracked.pose.pose.position.y
        text_marker.pose.position.z = tracked.pose.pose.position.z + tracked.dimensions.z + 0.3
        text_marker.scale.z = 0.28
        text_marker.color.r = 1.0
        text_marker.color.g = 1.0
        text_marker.color.b = 1.0
        text_marker.color.a = 1.0
        text_marker.text = "id={} label={}  v={:.2f} m/s".format(
            self.args.object_id, tracked.classification.label, speed
        )
        text_marker.lifetime = self.marker_lifetime
        return [box, arrow, text_marker]

    def publish_markers(self, message):
        markers = MarkerArray()
        markers.markers.append(self.route_marker(message.header))
        if message.objects:
            markers.markers.extend(self.object_markers(message.header, message.objects[0]))
        else:
            markers.markers.extend(self.delete_dynamic_markers(message.header))
        self.marker_publisher.publish(markers)

    def publish_once(self):
        now = rospy.Time.now()
        if now < self.last_time:
            rospy.logwarn("ROS time moved backwards; restarting the test route")
            self.start_time = now
        self.last_time = now
        elapsed = max(0.0, (now - self.start_time).to_sec())
        x, y, moving, reached_end = self.route_state(elapsed)

        message = TrackedObjectArray()
        message.header.stamp = now
        message.header.frame_id = self.args.frame_id
        if not (reached_end and self.args.end_behavior == "disappear"):
            message.objects.append(self.build_object(x, y, moving, elapsed))
        self.publisher.publish(message)
        self.publish_markers(message)

    def run(self):
        rate = rospy.Rate(self.args.rate)
        while not rospy.is_shutdown():
            self.publish_once()
            rate.sleep()


def main():
    args = parse_arguments()
    rospy.init_node("tracked_objects_test_publisher")
    try:
        publisher = TrackedObjectPublisher(args)
    except (ValueError, OverflowError, RuntimeError) as error:
        rospy.logfatal("Invalid tracked-object test configuration: %s", error)
        raise SystemExit(2)
    publisher.run()


if __name__ == "__main__":
    main()
