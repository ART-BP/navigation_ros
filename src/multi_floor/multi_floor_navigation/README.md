# multi_floor_navigation

同一个ROS节点中包含四个相互独立的模块：

- `TopologyGraph`：加载楼层、地图路径、节点和带方向的楼梯边。
- `TopologyPlanner`：使用Dijkstra生成`floor_msgs/NavigationRoute`。
- `RouteExecutor`：按段执行平地导航、楼梯action和切图状态。
- `MapSwitcher`：使用`map_server/change_map`切换地图并清理costmap。
- `StairController`：使用`map -> base_link` TF定位，分段执行中心线PID控制。

外部规划接口为`get_topology_plan` service，规划并执行接口为`navigate` action。
模块在进程内部直接共享只读`TopologyGraph`，地图路径不会进入ROS route消息。
楼梯路径保存在对应的有向边中；同一楼层的节点天然全部连通，规划时动态计算
节点之间的平层导航代价，不在图中保存O(N^2)条平层边。

`stair_executor_node`提供`/stair_navigation` action。楼梯路线配置三维点`[x, y, z]`；
z会被加载、存储并随Action传递，但当前不参与距离、方向、到达判断或PID控制。
第i段的起点yaw由第i点指向第i+1点的xy连线计算，配置中不保存yaw，最后一点也不需要yaw。
每一段开始前，它先主动将机器人的位置和计算出的yaw对齐到该段起点，再使用线速度PID和角速度PID跟踪起点与终点之间的
中心线。横向偏差进入减速区后会降低线速度，超过安全上限则立即停车并终止任务。
到达终点位置后不等待终点yaw，马上进入下一段的起点对齐。反馈进度为`1/N`至`N/N`。执行成功后
`RouteExecutor`才允许`MapSwitcher`切换到目标楼层。

楼梯控制循环每次通过TF查询`map -> base_link`。中心线跟踪使用机器人在当前线段上的
投影与前视点生成期望航向；线速度PID输入为终点距离，角速度PID输入为归一化航向误差。
航向误差过大时线速度置零，避免机器人侧向冲出中心线。所有配置yaw均使用弧度。
