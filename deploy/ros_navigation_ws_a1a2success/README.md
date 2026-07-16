# 导航程序启动说明

## 目录说明

部署目录：

```text
ros_navigation_ws
```

主导航脚本：

```text
GuideDog_dengxin.sh
```

## 启动步骤

### 1. 启动主导航

先启动机器人底盘、定位和感知程序，然后执行：

```bash
cd ~/ros_navigation_ws
./GuideDog_dengxin.sh
```

该脚本主要启动：

- 地图和 RViz；
- 点云转激光；
- `move_base` 和 TEB 局部规划器；
- `/cmd_vel` 到 `/ground_robot/cmd_vel` 的速度指令转换。

等待 RViz 中地图、机器人位置和导航状态正常后，再启动任务管理。

### 2. 启动任务管理

打开新终端：

```bash
source ~/ros_navigation_ws/install/setup.bash
```

A1 路线：从西溪智慧大厦出发，经过北门。

```bash
roslaunch road_yield a1.launch
```

A2 路线：从西溪智慧大厦出发，经过东北 2，到达南门。

```bash
roslaunch road_yield a2.launch
```

> A1 和 A2 不能同时启动。切换路线前，应先按 `Ctrl+C` 停止当前任务。

## 主要功能

### A1

- 按预设航点连续导航；
- 在指定航点自动进入台阶模式；
- 台阶结束后恢复正常导航参数。

### A2

- 按预设航点连续导航；
- 通过 `/tracked_objects` 检测道路来车；
- 检测到来车后暂停当前路线并前往靠边点；
- 道路恢复安全后继续原路线。

### 任务异常处理

- 导航目标失败时自动重试；
- 避让点不可达时自动尝试其他候选点；
- 多次失败后进入 `SAFE_STOP`，需要排查故障并重新启动任务。

## 注意事项

- 脚本默认工作空间路径为 `~/ros_navigation_ws`；
- 默认地图路径为：

  ```text
  /home/z/slam_ws/data/GDOG_dengxin_20260626_multi_session_map_data/map_data/2d_map.yaml
  ```

- A2 来车避让依赖 `/tracked_objects`；
- 停止时先关闭 A1/A2 任务，再关闭主导航。
