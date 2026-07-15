#!/usr/bin/env python3
import argparse
import csv
import math
import os
import threading

import actionlib
import rospy
import tf2_ros
import yaml
from actionlib_msgs.msg import GoalStatus
from dynamic_reconfigure.client import Client as DynClient
from geometry_msgs.msg import Quaternion
from move_base_msgs.msg import MoveBaseAction, MoveBaseGoal
from std_msgs.msg import Bool


GOAL_STATE_NAMES = {
    GoalStatus.PENDING: "PENDING",
    GoalStatus.ACTIVE: "ACTIVE",
    GoalStatus.PREEMPTED: "PREEMPTED",
    GoalStatus.SUCCEEDED: "SUCCEEDED",
    GoalStatus.ABORTED: "ABORTED",
    GoalStatus.REJECTED: "REJECTED",
    GoalStatus.PREEMPTING: "PREEMPTING",
    GoalStatus.RECALLING: "RECALLING",
    GoalStatus.RECALLED: "RECALLED",
    GoalStatus.LOST: "LOST",
}


def yaw_to_quaternion(yaw):
    half = yaw * 0.5
    return Quaternion(x=0.0, y=0.0, z=math.sin(half), w=math.cos(half))


def parse_csv_number(value, column, row_number):
    if value is None or not str(value).strip():
        raise ValueError("CSV row {} has an empty '{}' value".format(row_number, column))
    try:
        number = float(value)
    except (TypeError, ValueError):
        raise ValueError(
            "CSV row {} has an invalid '{}' value: {!r}".format(row_number, column, value)
        )
    if not math.isfinite(number):
        raise ValueError(
            "CSV row {} has a non-finite '{}' value: {!r}".format(row_number, column, value)
        )
    return number


def load_waypoint_csv(path, default_frame):
    waypoints = []
    with open(path, "r", encoding="utf-8-sig", newline="") as f:
        reader = csv.DictReader(f)
        if reader.fieldnames is None:
            raise ValueError("Waypoint CSV has no header: {}".format(path))

        reader.fieldnames = [name.strip() if name is not None else "" for name in reader.fieldnames]
        required_columns = {"mx", "my", "yaw"}
        missing = sorted(required_columns.difference(reader.fieldnames))
        if missing:
            raise ValueError(
                "Waypoint CSV is missing required columns {}: {}".format(
                    ", ".join(missing), path
                )
            )

        for row_number, row in enumerate(reader, start=2):
            if None in row:
                raise ValueError("CSV row {} has more values than the header".format(row_number))
            if not any(value is not None and str(value).strip() for value in row.values()):
                continue

            waypoint = {
                "name": "waypoint_{}".format(len(waypoints) + 1),
                "x": parse_csv_number(row.get("mx"), "mx", row_number),
                "y": parse_csv_number(row.get("my"), "my", row_number),
                "yaw_deg": parse_csv_number(row.get("yaw"), "yaw", row_number),
                "frame_id": default_frame,
            }
            command = (row.get("cmd") or "").strip()
            if command:
                waypoint["cmd"] = command
            waypoints.append(waypoint)

    if not waypoints:
        raise ValueError("Waypoint CSV contains no waypoints: {}".format(path))
    return waypoints


def load_yaml(path):
    config_path = os.path.abspath(os.path.expanduser(path))
    with open(config_path, "r") as f:
        data = yaml.safe_load(f) or {}

    waypoint_file = data.get("waypoint_file")
    if waypoint_file:
        if not isinstance(waypoint_file, str):
            raise ValueError("YAML 'waypoint_file' must be a file name or path")
        waypoint_path = os.path.expanduser(waypoint_file)
        if not os.path.isabs(waypoint_path):
            waypoint_path = os.path.join(os.path.dirname(config_path), waypoint_path)
        waypoint_path = os.path.abspath(waypoint_path)
        data["waypoints"] = load_waypoint_csv(waypoint_path, data.get("frame_id", "map"))
        data["_waypoint_file_path"] = waypoint_path
    elif "waypoints" not in data or not isinstance(data["waypoints"], list):
        raise ValueError("YAML must contain 'waypoint_file' or a legacy 'waypoints' list")

    if not data["waypoints"]:
        raise ValueError("No waypoints were loaded")

    index_base = int(data.get("stair_index_base", 1))
    stair_indices = list(data.get("stair_indices", []))
    stair_indices.extend(data.get("stair_waypoint_indices", []))
    first_index = index_base
    last_index = index_base + len(data["waypoints"]) - 1
    invalid_indices = [int(value) for value in stair_indices if not first_index <= int(value) <= last_index]
    if invalid_indices:
        raise ValueError(
            "Stair waypoint indices are outside [{}, {}]: {}".format(
                first_index, last_index, invalid_indices
            )
        )
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


def normalize_frame_id(frame_id):
    if not frame_id:
        return frame_id
    if frame_id.startswith("/"):
        return frame_id[1:]
    return frame_id


def goal_state_name(state):
    return GOAL_STATE_NAMES.get(state, str(state))


class RobotPoseLookup:
    def __init__(self, base_frame, timeout):
        self.base_frame = normalize_frame_id(base_frame)
        self.timeout = float(timeout)
        self.tf_buffer = tf2_ros.Buffer()
        self.tf_listener = tf2_ros.TransformListener(self.tf_buffer)
        self.last_warn_time = 0.0
        self.warn_interval = 2.0

    def distance_to_goal(self, goal):
        goal_frame = normalize_frame_id(goal.target_pose.header.frame_id)
        try:
            transform = self.tf_buffer.lookup_transform(
                goal_frame,
                self.base_frame,
                rospy.Time(0),
                rospy.Duration(self.timeout),
            )
        except Exception as exc:
            now = rospy.get_time()
            if now - self.last_warn_time >= self.warn_interval:
                self.last_warn_time = now
                rospy.logwarn(
                    "Could not calculate distance to goal in frame '%s' from base frame '%s': %s",
                    goal.target_pose.header.frame_id,
                    self.base_frame,
                    exc,
                )
            return None

        current = transform.transform.translation
        target = goal.target_pose.pose.position
        return math.hypot(current.x - target.x, current.y - target.y)


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


def wait_for_goal_once(move_base, name, timeout):
    if timeout > 0.0:
        finished = move_base.wait_for_result(rospy.Duration(timeout))
    else:
        finished = move_base.wait_for_result()

    if not finished:
        move_base.cancel_goal()
        move_base.wait_for_result(rospy.Duration(1.0))
        return False, "timed out after {:.1f}s".format(timeout)

    state = move_base.get_state()
    if state == GoalStatus.SUCCEEDED:
        return True, "succeeded"
    return False, "state={}".format(goal_state_name(state))


def send_goal_with_retry_policy(move_base, goal, name, timeout, pose_lookup, retry_distance, retry_count):
    max_attempts = 1 + max(0, int(retry_count))
    retry_distance = float(retry_distance)

    for attempt in range(1, max_attempts + 1):
        rospy.loginfo("Sending goal attempt %d/%d: %s", attempt, max_attempts, name)
        goal.target_pose.header.stamp = rospy.Time.now()
        move_base.send_goal(goal)
        reached, reason = wait_for_goal_once(move_base, name, timeout)
        if reached:
            return True

        distance = pose_lookup.distance_to_goal(goal)
        if distance is None:
            rospy.logwarn(
                "Goal failed (%s), but current distance is unavailable; advancing to next waypoint: %s",
                reason,
                name,
            )
            return False

        if distance <= retry_distance:
            rospy.logwarn(
                "Goal failed (%s), current distance %.3f m <= %.3f m; advancing to next waypoint: %s",
                reason,
                distance,
                retry_distance,
                name,
            )
            return False

        if attempt >= max_attempts:
            rospy.logwarn(
                "Goal failed (%s), current distance %.3f m > %.3f m after %d attempts; advancing to next waypoint: %s",
                reason,
                distance,
                retry_distance,
                attempt,
                name,
            )
            return False

        rospy.logwarn(
            "Goal failed (%s), current distance %.3f m > %.3f m; retrying goal: %s",
            reason,
            distance,
            retry_distance,
            name,
        )

    return False


def send_goal_until_close(move_base, goal, name, timeout, pose_lookup, advance_distance, check_rate, retry_distance, retry_count):
    max_attempts = 1 + max(0, int(retry_count))
    advance_distance = float(advance_distance)
    retry_distance = float(retry_distance)
    check_period = 1.0 / check_rate if check_rate > 0.0 else 0.2

    for attempt in range(1, max_attempts + 1):
        rospy.loginfo("Sending pass-through goal attempt %d/%d: %s", attempt, max_attempts, name)
        goal.target_pose.header.stamp = rospy.Time.now()
        move_base.send_goal(goal)
        start_time = rospy.get_time()
        failure_reason = None

        while not rospy.is_shutdown():
            if advance_distance > 0.0:
                distance = pose_lookup.distance_to_goal(goal)
                if distance is not None and distance <= advance_distance:
                    rospy.loginfo(
                        "Current distance %.3f m <= %.3f m; sending next waypoint: %s",
                        distance,
                        advance_distance,
                        name,
                    )
                    return "advanced"

            if move_base.wait_for_result(rospy.Duration(check_period)):
                state = move_base.get_state()
                if state == GoalStatus.SUCCEEDED:
                    return "reached"
                failure_reason = "state={}".format(goal_state_name(state))
                break

            now = rospy.get_time()
            if timeout > 0.0 and (now - start_time) >= timeout:
                move_base.cancel_goal()
                move_base.wait_for_result(rospy.Duration(1.0))
                failure_reason = "timed out after {:.1f}s".format(timeout)
                break

        if rospy.is_shutdown():
            move_base.cancel_goal()
            raise rospy.ROSInterruptException("Interrupted while waiting for goal: {}".format(name))

        distance = pose_lookup.distance_to_goal(goal)
        if distance is None:
            rospy.logwarn(
                "Pass-through goal failed (%s), but current distance is unavailable; moving to next waypoint: %s",
                failure_reason,
                name,
            )
            return "failed"

        if distance <= retry_distance:
            rospy.logwarn(
                "Pass-through goal failed (%s), current distance %.3f m <= %.3f m; moving to next waypoint: %s",
                failure_reason,
                distance,
                retry_distance,
                name,
            )
            return "failed"

        if attempt >= max_attempts:
            rospy.logwarn(
                "Pass-through goal failed (%s), current distance %.3f m > %.3f m after %d attempts; moving to next waypoint: %s",
                failure_reason,
                distance,
                retry_distance,
                attempt,
                name,
            )
            return "failed"

        rospy.logwarn(
            "Pass-through goal failed (%s), current distance %.3f m > %.3f m; retrying goal: %s",
            failure_reason,
            distance,
            retry_distance,
            name,
        )

    return "failed"


def run_waypoints(config):
    move_base_action = config.get("move_base_action", "/move_base")
    default_frame = config.get("frame_id", "map")
    goal_timeout = float(config.get("goal_timeout", 120.0))
    wait_server_timeout = float(config.get("wait_server_timeout", 10.0))
    continue_on_failure = bool(config.get("continue_on_failure", False))
    advance_distance = float(
        config.get(
            "near_goal_advance_distance",
            config.get("near_goal_skip_distance", config.get("skip_distance", 5.0)),
        )
    )
    advance_check_rate = float(config.get("advance_check_rate", 5.0))
    failure_retry_distance = float(config.get("failure_retry_distance", advance_distance))
    failure_retry_count = int(config.get("failure_retry_count", 2))
    robot_base_frame = config.get("robot_base_frame", "base_link")
    pose_lookup_timeout = float(config.get("pose_lookup_timeout", 0.1))

    stair_topic = config.get("stair_mode_topic", "/stair_mode")
    stair_rate = float(config.get("stair_publish_rate", 10.0))
    teb_ns = config.get("teb_reconfigure_ns", "/move_base/TebLocalPlannerROS")
    dyn_timeout = float(config.get("dynamic_reconfigure_timeout", 5.0))
    stair_teb_params = config.get("stair_teb_params", {})

    stair_pub = StairModePublisher(stair_topic, stair_rate)
    pose_lookup = RobotPoseLookup(robot_base_frame, pose_lookup_timeout)
    move_base = actionlib.SimpleActionClient(move_base_action, MoveBaseAction)

    if config.get("_waypoint_file_path"):
        rospy.loginfo(
            "Loaded %d waypoints from CSV (yaw unit: degrees): %s",
            len(config["waypoints"]),
            config["_waypoint_file_path"],
        )

    rospy.loginfo("Waiting for move_base action server: %s", move_base_action)
    if not move_base.wait_for_server(rospy.Duration(wait_server_timeout)):
        raise RuntimeError("move_base action server is not available: {}".format(move_base_action))

    try:
        for index, wp in enumerate(config["waypoints"]):
            wp = normalize_waypoint(wp, index, default_frame)
            name = wp.get("name", "waypoint_{}".format(index))
            is_stair = waypoint_is_stair(config, wp, index)
            is_last = index == len(config["waypoints"]) - 1
            timeout = float(wp.get("timeout", goal_timeout))
            waypoint_advance_distance = float(
                wp.get(
                    "near_goal_advance_distance",
                    wp.get("near_goal_skip_distance", wp.get("skip_distance", advance_distance)),
                )
            )
            waypoint_check_rate = float(wp.get("advance_check_rate", advance_check_rate))
            waypoint_retry_distance = float(wp.get("failure_retry_distance", failure_retry_distance))
            waypoint_retry_count = int(wp.get("failure_retry_count", failure_retry_count))
            guard = TebParamGuard(teb_ns, stair_teb_params, dyn_timeout)

            rospy.loginfo("Handling goal %d/%d: %s stair=%s", index + 1, len(config["waypoints"]), name, is_stair)
            try:
                goal = make_goal(wp, default_frame)

                if is_stair:
                    guard.apply()
                    stair_pub.start()
                    rospy.sleep(1)

                if is_stair or is_last:
                    reached = send_goal_with_retry_policy(
                        move_base,
                        goal,
                        name,
                        timeout,
                        pose_lookup,
                        waypoint_retry_distance,
                        waypoint_retry_count,
                    )
                    if reached:
                        rospy.loginfo("Goal reached: %s", name)
                    else:
                        rospy.logwarn("Goal not reached after retry policy; moving to next waypoint: %s", name)
                else:
                    result = send_goal_until_close(
                        move_base,
                        goal,
                        name,
                        timeout,
                        pose_lookup,
                        waypoint_advance_distance,
                        waypoint_check_rate,
                        waypoint_retry_distance,
                        waypoint_retry_count,
                    )
                    if result == "advanced":
                        rospy.loginfo("Goal passed; next waypoint will preempt it: %s", name)
                    elif result == "reached":
                        rospy.loginfo("Goal reached before pass-through threshold: %s", name)
                    else:
                        rospy.logwarn("Goal not reached after retry policy; moving to next waypoint: %s", name)
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
    parser = argparse.ArgumentParser(
        description="Run move_base waypoints loaded from a CSV configured by YAML."
    )
    parser.add_argument("config", help="YAML file containing waypoint_file and stair indices")
    return parser.parse_args(rospy.myargv()[1:])


def main():
    args = parse_args()
    rospy.init_node("stair_waypoint_runner")
    config = load_yaml(args.config)
    run_waypoints(config)


if __name__ == "__main__":
    main()
