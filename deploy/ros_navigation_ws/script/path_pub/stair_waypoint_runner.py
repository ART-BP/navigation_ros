#!/usr/bin/env python3
import argparse
import math
import threading

import actionlib
import rospy
import yaml
from actionlib_msgs.msg import GoalStatus
from dynamic_reconfigure.client import Client as DynClient
from geometry_msgs.msg import Quaternion
from move_base_msgs.msg import MoveBaseAction, MoveBaseGoal
from std_msgs.msg import Bool


def yaw_to_quaternion(yaw):
    half = yaw * 0.5
    return Quaternion(x=0.0, y=0.0, z=math.sin(half), w=math.cos(half))


def load_yaml(path):
    with open(path, "r") as f:
        data = yaml.safe_load(f) or {}
    if "waypoints" not in data or not isinstance(data["waypoints"], list):
        raise ValueError("YAML must contain a 'waypoints' list")
    return data


class StairModePublisher:
    def __init__(self, topic, rate_hz):
        self.pub = rospy.Publisher(topic, Bool, queue_size=1)
        self.rate_hz = rate_hz
        self.stop_event = threading.Event()
        self.thread = None

    def start(self):
        self.stop()
        self.stop_event.clear()
        self.thread = threading.Thread(target=self._run)
        self.thread.daemon = True
        self.thread.start()

    def stop(self):
        if self.thread is not None:
            self.stop_event.set()
            self.thread.join(1.0)
            self.thread = None
        for _ in range(5):
            self.pub.publish(Bool(data=False))
            rospy.sleep(0.02)

    def _run(self):
        rate = rospy.Rate(self.rate_hz)
        msg = Bool(data=True)
        while not rospy.is_shutdown() and not self.stop_event.is_set():
            self.pub.publish(msg)
            rate.sleep()


class TebParamGuard:
    def __init__(self, namespace, overrides, timeout):
        self.namespace = namespace
        self.overrides = dict(overrides or {})
        self.timeout = timeout
        self.client = None
        self.original = None

    def apply(self):
        if not self.overrides:
            return
        self.client = DynClient(self.namespace, timeout=self.timeout)
        current = self.client.get_configuration(timeout=self.timeout)
        missing = [key for key in self.overrides if key not in current]
        if missing:
            raise KeyError("TEB dynamic_reconfigure keys not found: {}".format(", ".join(missing)))
        self.original = {key: current[key] for key in self.overrides}
        rospy.loginfo("Applying stair TEB params: %s", self.overrides)
        self.client.update_configuration(self.overrides)

    def restore(self):
        if self.client is None or self.original is None:
            return
        rospy.loginfo("Restoring TEB params: %s", self.original)
        self.client.update_configuration(self.original)
        self.original = None


def normalize_waypoint(wp, index, default_frame):
    if isinstance(wp, (list, tuple)):
        if len(wp) < 2:
            raise ValueError("Waypoint list #{} must contain at least [x, y]".format(index + 1))
        normalized = {
            "name": "waypoint_{}".format(index + 1),
            "x": wp[0],
            "y": wp[1],
            "frame_id": default_frame,
        }
        if len(wp) >= 3:
            normalized["yaw"] = wp[2]
        if len(wp) >= 4:
            normalized["stair"] = wp[3]
        return normalized

    if isinstance(wp, dict):
        normalized = dict(wp)
        normalized.setdefault("name", "waypoint_{}".format(index + 1))
        normalized.setdefault("frame_id", default_frame)
        return normalized

    raise ValueError("Waypoint #{} must be a dict or list".format(index + 1))


def waypoint_is_stair(config, wp, index):
    if "stair" in wp:
        return bool(wp["stair"])

    stair_names = set(config.get("stair_names", []))
    stair_names.update(config.get("stair_waypoint_names", []))
    if wp.get("name") in stair_names:
        return True

    index_base = int(config.get("stair_index_base", 1))
    stair_indices = set(int(i) for i in config.get("stair_indices", []))
    stair_indices.update(int(i) for i in config.get("stair_waypoint_indices", []))
    return (index + index_base) in stair_indices


def make_goal(wp, default_frame):
    frame_id = wp.get("frame_id", default_frame)
    yaw = wp.get("yaw")
    if yaw is None:
        yaw = math.radians(float(wp.get("yaw_deg", 0.0)))
    goal = MoveBaseGoal()
    goal.target_pose.header.frame_id = frame_id
    goal.target_pose.header.stamp = rospy.Time.now()
    goal.target_pose.pose.position.x = float(wp["x"])
    goal.target_pose.pose.position.y = float(wp["y"])
    goal.target_pose.pose.position.z = float(wp.get("z", 0.0))
    goal.target_pose.pose.orientation = yaw_to_quaternion(float(yaw))
    return goal


def run_waypoints(config):
    move_base_action = config.get("move_base_action", "/move_base")
    default_frame = config.get("frame_id", "map")
    goal_timeout = float(config.get("goal_timeout", 120.0))
    wait_server_timeout = float(config.get("wait_server_timeout", 10.0))
    continue_on_failure = bool(config.get("continue_on_failure", False))

    stair_topic = config.get("stair_mode_topic", "/stair_mode")
    stair_rate = float(config.get("stair_publish_rate", 10.0))
    teb_ns = config.get("teb_reconfigure_ns", "/move_base/TebLocalPlannerROS")
    dyn_timeout = float(config.get("dynamic_reconfigure_timeout", 5.0))
    stair_teb_params = config.get("stair_teb_params", {})

    stair_pub = StairModePublisher(stair_topic, stair_rate)
    move_base = actionlib.SimpleActionClient(move_base_action, MoveBaseAction)

    rospy.loginfo("Waiting for move_base action server: %s", move_base_action)
    if not move_base.wait_for_server(rospy.Duration(wait_server_timeout)):
        raise RuntimeError("move_base action server is not available: {}".format(move_base_action))

    try:
        for index, wp in enumerate(config["waypoints"]):
            wp = normalize_waypoint(wp, index, default_frame)
            name = wp.get("name", "waypoint_{}".format(index))
            is_stair = waypoint_is_stair(config, wp, index)
            timeout = float(wp.get("timeout", goal_timeout))
            guard = TebParamGuard(teb_ns, stair_teb_params, dyn_timeout)

            rospy.loginfo("Sending goal %d/%d: %s stair=%s", index + 1, len(config["waypoints"]), name, is_stair)
            try:
                if is_stair:
                    guard.apply()
                    stair_pub.start()
                    rospy.sleep(1.0)

                goal = make_goal(wp, default_frame)
                move_base.send_goal(goal)
                finished = move_base.wait_for_result(rospy.Duration(timeout))
                if not finished:
                    move_base.cancel_goal()
                    raise RuntimeError("Goal timed out: {}".format(name))

                state = move_base.get_state()
                if state != GoalStatus.SUCCEEDED:
                    raise RuntimeError("Goal failed: {} state={}".format(name, state))

                rospy.loginfo("Goal reached: %s", name)
            except Exception as exc:
                rospy.logerr("%s", exc)
                if not continue_on_failure:
                    raise
            finally:
                if is_stair:
                    stair_pub.stop()
                    guard.restore()
    finally:
        stair_pub.stop()


def parse_args():
    parser = argparse.ArgumentParser(description="Run move_base waypoints with optional stair mode segments.")
    parser.add_argument("config", help="Waypoint YAML file")
    return parser.parse_args(rospy.myargv()[1:])


def main():
    args = parse_args()
    rospy.init_node("stair_waypoint_runner")
    config = load_yaml(args.config)
    run_waypoints(config)


if __name__ == "__main__":
    main()
