# road_yield

This ROS1 node sends a configured move_base route and preempts it when a
configured vehicle is detected inside the ahead portion of a road region. It
then enters stair mode and navigates to the nearest eligible shelter. A fixed
wait timer starts when the shelter goal succeeds and is not reset by vehicle
detections. The interrupted route resumes only when that timer has elapsed and
five new consecutive perception messages report neither an oncoming vehicle
ahead nor a configured vehicle behind.

## Code structure

- `config_loader`: reads and validates YAML and the three CSV files.
- `road_geometry`: provides the pure road-region and centerline calculations.
- `pose_provider`: owns all TF lookup and point transformation details.
- `vehicle_monitor`: filters tracked objects and publishes a thread-safe
  ahead-vehicle snapshot to the state machine.
- `navigation_client`: owns the `move_base` action lifecycle, timeout and
  result synchronization.
- `stair_controller`: owns `/stair_mode` publishing and temporary TEB
  dynamic-reconfigure values.
- `road_yield_manager`: contains only route/yield state transitions and
  retry policy.
- `road_yield_node`: parses the ROS parameter and starts the manager.

## Patrol stair waypoints

Patrol stair goals can be marked in either of two ways:

- write `stair` in the patrol CSV row's `cmd` column; or
- add its waypoint number to YAML `stair_indices` (1-based by default through
  `stair_index_base`; `stair_waypoint_indices` is accepted as an alias).

Before sending a marked patrol goal, the manager enables `/stair_mode`, applies
`stair_teb_params`, and waits `stair_warmup_time`. Unlike a normal intermediate
patrol point, a stair point is never advanced merely because the robot entered
`near_goal_advance_distance`; it waits for the `move_base` result. Stair mode
and the original TEB values are restored after the point succeeds or is
abandoned by the route failure policy. Empty `cmd` values and an empty
`stair_indices` list preserve normal patrol behavior.

## Patrol-only mode

`route_waypoint_file` is mandatory. `avoidance_waypoint_file` and
`road_boundary_file` are an optional pair. If neither file is configured, or
both configured files are absent, road-yield perception is disabled and the
node runs the patrol route normally. Patrol stair markers remain active in
this mode, and startup never waits for tracked-object perception. If only one
of the two road-yield files is available, startup fails to expose the
incomplete deployment instead of silently disabling yield behavior.

## Road-region convention

`road_cross_sections` is a connected sequence of left/right curb pairs. Each
adjacent pair forms a road quadrilateral. The forward direction is recalculated
from the current robot position to the active patrol goal, so the same boundary
data also works when the route turns around. A vehicle triggers yielding only
when it is:

1. classified as one of `vehicle_labels`;
2. inside one of the road quadrilaterals; and
3. ahead of the robot along the robot-to-goal direction by `ahead_margin`; and
4. stationary, moving opposite to the robot-to-goal direction, or has no valid
   velocity.

The latest `detection_window_size` perception messages form a sliding window.
Yielding starts only when at least `detection_required_count` messages in that
window contain an oncoming vehicle. With the default 5 Hz perception settings,
this is three positive detections among five consecutive messages.

When yielding starts, the node retains the nearest
`avoidance_candidate_limit` eligible shelters for that event. While driving to
the selected shelter, the robot-to-goal distance must improve by at least
`avoidance_min_progress` within every `avoidance_progress_timeout` interval.
Otherwise the goal is canceled, the shelter is blacklisted for the current
yield event, and the next-nearest candidate is sent immediately. A `move_base`
failure or timeout causes the same switch instead of retrying the blocked
shelter. If all retained candidates fail, the manager enters `SAFE_STOP`.

The longitudinal distance is calculated with a dot product against the unit
robot-to-goal vector. A goal change invalidates the previous perception
snapshot, so a fresh tracked-object message must be evaluated with the new
direction before it can trigger or clear a yield.

The loader is isolated in `src/config_loader.cpp`. It reads three independent
CSV files configured by `route_waypoint_file`, `avoidance_waypoint_file`, and
`road_boundary_file`. `waypoint_line` in the boundary CSV is the physical line
number in the patrol CSV, including its header line.

## Run

```bash
roslaunch road_yield road_yield.launch \
  config:=/absolute/path/to/road_yield.yaml
```

The node can wait for a fresh `/tracked_objects` message before starting when
`require_perception_before_navigation` is enabled. With the supplied setting
disabled, patrol continues during a perception interruption and emits a
throttled warning every `perception_warning_interval` seconds while traversing
the road-yield detection section. While waiting at a shelter, perception timeout
resets the consecutive-clear count and emits the same throttled warning. Route
navigation resumes only after the minimum wait time and
`clear_required_count` new consecutive messages contain neither an oncoming
vehicle ahead nor a configured vehicle behind. Classification probability is
not used because the current publisher does not populate it.
