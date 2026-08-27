# multi_floor_navigation

同一个ROS节点中包含四个相互独立的模块：

- `TopologyGraph`：加载楼层、地图路径、节点和带方向的楼梯边。
- `TopologyPlanner`：使用Dijkstra生成`floor_msgs/NavigationRoute`。
- `RouteExecutor`：按段执行平地导航、楼梯action和切图状态。
- `MapSwitcher`：使用`map_server/change_map`切换地图并清理costmap。
- `StairController`：使用`map -> base_link` TF定位，分段执行中心线PID控制。

外部规划接口为`get_topology_plan` service，规划并执行接口为`navigate` action。
这两个接口只接收目标位姿和目标楼层。节点在收到请求时通过`map -> base_link` TF
读取实际起点，并以`MapSwitcher::current_floor()`作为起始楼层；调用方不再发送起点。
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
楼梯入口执行位置和方向的完整对齐；中间共享点由上一段确认到达后，下一段只对齐新方向，
不再因轻微越点而返回旧点。机器人到达或越过当前段终点垂线时立即结束该段；若定位跳变
导致越过距离超过中心线偏差上限，则停车并中止任务，不执行原地掉头追回。
楼梯控制同时监控`/pseudo_cloud_base`：在`base_link`正前方矩形内，上楼时若存在
`z > 0.1m`的点、下楼时若存在`z > -0.1m`的点，则速度输出被置零；检测区域恢复为空后
自动继续原有对齐或中心线控制。上下楼方向由Action的目标楼层与起始楼层比较得到。

启用`Go2WMotionManager`后，节点启动时先切到平地模式0。平层段开始前会再次确认模式0；
到达楼梯口后先取消并等待`move_base`停止，再切换到台阶模式1并启动楼梯action。
楼梯成功到达目标楼层后切回模式0；楼梯失败或取消时只执行`StopMove`并保留模式1，
避免机器人仍停在台阶上时自动切回普通步态。模式切换期间action反馈状态为`SWITCHING_MODE`。
可通过`motion_manager_enabled:=false`关闭运行时模式管理，通信网卡由`network_interface`指定。

发送导航目标（示例目标为B5的`x=10.0, y=5.0, yaw=0`）：

```bash
rostopic pub -1 /navigate/goal floor_msgs/MultiFloorNavigationActionGoal "
header: {stamp: now}
goal_id: {stamp: now, id: 'manual_goal_001'}
goal:
  goal:
    header: {stamp: now, frame_id: 'map'}
    pose:
      position: {x: 10.0, y: 5.0, z: 0.0}
      orientation: {x: 0.0, y: 0.0, z: 0.0, w: 1.0}
  goal_floor: 10500
"
```

`goal_floor`小于0时表示当前楼层。同一套全局三维定位必须持续发布
`map -> base_link`，`initial_floor`必须与启动时实际所在楼层一致。

楼梯控制循环每次通过TF查询`map -> base_link`。中心线跟踪使用机器人在当前线段上的
投影与前视点生成期望航向；线速度PID输入为终点距离，角速度PID输入为归一化航向误差。
航向误差过大时线速度置零，避免机器人侧向冲出中心线。所有配置yaw均使用弧度。
