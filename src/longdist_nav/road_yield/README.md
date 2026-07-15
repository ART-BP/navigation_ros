# road_yield

This ROS1 node sends a configured move_base route and preempts it when a
configured vehicle is detected inside the ahead portion of a road region. It
then enters stair mode and navigates to the nearest eligible shelter. A fixed
wait timer starts when the shelter goal succeeds and is not reset by vehicle
detections. The interrupted route resumes only when that timer has elapsed and
fresh perception reports no vehicle in the forward road region.

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

## Road-region convention

`road_cross_sections` is a connected sequence of left/right curb pairs. Each
adjacent pair forms a road quadrilateral. The forward direction is recalculated
from the current robot position to the active patrol goal, so the same boundary
data also works when the route turns around. A vehicle triggers yielding only
when it is:

1. classified as one of `vehicle_labels`;
2. inside one of the road quadrilaterals; and
3. ahead of the robot along the robot-to-goal direction by `ahead_margin`.

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

The node intentionally waits for a fresh `/tracked_objects` message before
starting by default. While waiting at a shelter, perception timeout is treated
as unsafe rather than as a clear forward road region. Classification
probability is not used because the current publisher does not populate it.
