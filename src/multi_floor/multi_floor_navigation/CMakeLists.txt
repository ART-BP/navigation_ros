cmake_minimum_required(VERSION 3.0.2)
project(multi_floor_navigation)

add_compile_options(-Wall -Wextra -Wpedantic)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(
  unitree_sdk2_DIR
  /opt/unitree_robotics/lib/cmake/unitree_sdk2
)

find_package(catkin REQUIRED COMPONENTS
  actionlib
  floor_msgs
  geometry_msgs
  move_base_msgs
  nav_msgs
  roscpp
  roslib
  sensor_msgs
  std_srvs
  tf2
  tf2_geometry_msgs
  tf2_ros
)
find_package(yaml-cpp REQUIRED)

find_package(unitree_sdk2 REQUIRED CONFIG)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES multi_floor_navigation_core
  CATKIN_DEPENDS
    actionlib
    floor_msgs
    geometry_msgs
    move_base_msgs
    nav_msgs
    roscpp
    roslib
    sensor_msgs
    std_srvs
    tf2
    tf2_geometry_msgs
    tf2_ros
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${YAML_CPP_INCLUDE_DIRS}
)
# =========================
# mode_switch_node
# =========================
add_executable(mode_switch_node
  src/go2w_motion_manager.cpp
)

target_link_libraries(mode_switch_node
  ${catkin_LIBRARIES}
  unitree_sdk2
)

add_dependencies(mode_switch_node
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS}
)

add_library(multi_floor_navigation_core
  src/go2w_motion_manager.cpp
  src/map_switcher.cpp
  src/route_executor.cpp
  src/stair_action_client.cpp
  src/stair_controller.cpp
  src/topology_map.cpp
  src/topology_planner.cpp
)
add_dependencies(multi_floor_navigation_core
  ${catkin_EXPORTED_TARGETS}
)
target_link_libraries(multi_floor_navigation_core
  ${catkin_LIBRARIES}
  unitree_sdk2
  yaml-cpp
)

add_executable(multi_floor_navigation_node
  src/multi_floor_navigation_node.cpp
)
add_dependencies(multi_floor_navigation_node
  ${catkin_EXPORTED_TARGETS}
)
target_link_libraries(multi_floor_navigation_node
  multi_floor_navigation_core
  ${catkin_LIBRARIES}
)

add_executable(stair_executor_node
  src/stair_executor_node.cpp
)
add_dependencies(stair_executor_node
  ${catkin_EXPORTED_TARGETS}
)
target_link_libraries(stair_executor_node
  multi_floor_navigation_core
  ${catkin_LIBRARIES}
)

add_executable(test_map
  src/test_map_switcher.cpp
)
add_dependencies(test_map
  ${catkin_EXPORTED_TARGETS}
)
target_link_libraries(test_map
  multi_floor_navigation_core
  ${catkin_LIBRARIES}
)
install(TARGETS multi_floor_navigation_core multi_floor_navigation_node stair_executor_node mode_switch_node test_map
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
  FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY config launch maps
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)
