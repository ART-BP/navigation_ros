# road_yield

This ROS1 node sends a configured move_base route and preempts it when a
configured vehicle is detected inside the ahead portion of a road region. It
then enters stair mode and navigates to the nearest eligible shelter. A fixed
wait timer starts when the shelter goal succeeds and is not reset by vehicle
detections. The interrupted route resumes only when that timer has elapsed and
fresh perception reports no vehicle in the forward road region.

## Road-region convention

`road_cross_sections` is a sequence of left/right curb pairs ordered in the
robot's intended direction of travel. Each adjacent pair forms a road
quadrilateral. Both robot and vehicle are projected onto the centerline made by
the midpoints of these pairs. A vehicle triggers yielding only when it is:

1. classified as one of `vehicle_labels`;
2. inside one of the road quadrilaterals; and
3. farther along the ordered centerline than the robot by `ahead_margin`.

This centerline-station check is what prevents a detected vehicle behind the
robot from triggering another yield, including on curved roads.

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
