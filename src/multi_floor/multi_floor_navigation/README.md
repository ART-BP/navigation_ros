# multi_floor_navigation

同一个ROS节点中包含四个相互独立的模块：

- `TopoGraph`：加载楼层、地图路径、节点、边和楼梯顺序点。
- `TopologyPlanner`：使用Dijkstra生成`floor_msgs/NavigationRoute`。
- `RouteExecutor`：按段执行平地导航、楼梯action和切图状态。
- `MapSwitcher`：使用`map_server/change_map`切换地图并清理costmap。
- `StairController`：使用`map -> base_link` TF定位，分段执行中心线PID控制。

外部规划接口为`get_topology_plan` service，规划并执行接口为`navigate` action。
模块在进程内部直接共享只读`TopoGraph`，地图路径不会进入ROS route消息。

`stair_executor_node`提供`/stair_navigation` action。它首先检查机器人是否位于
第一个配置点且yaw误差在阈值内，随后逐段使用线速度PID和角速度PID跟踪配置点之间
的中心线，到点后对齐目标yaw。反馈进度为`1/N`至`N/N`。执行成功后
`RouteExecutor`才允许`MapSwitcher`切换到目标楼层。

楼梯控制循环每次通过TF查询`map -> base_link`。中心线跟踪使用机器人在当前线段上的
投影与前视点生成期望航向；线速度PID输入为终点距离，角速度PID输入为归一化航向误差。
航向误差过大时线速度置零，避免机器人侧向冲出中心线。所有配置yaw均使用弧度。
