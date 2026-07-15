cmake_minimum_required(VERSION 3.0.2)
project(road_yield)

add_compile_options(-Wall -Wextra -Wpedantic)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(catkin REQUIRED COMPONENTS
  actionlib
  dynamic_obstacles
  dynamic_reconfigure
  geometry_msgs
  move_base_msgs
  roscpp
  std_msgs
  teb_local_planner
  tf2
  tf2_geometry_msgs
  tf2_ros
)
find_package(yaml-cpp REQUIRED)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES road_geometry road_yield_config road_yield_core
  CATKIN_DEPENDS
    actionlib
    dynamic_obstacles
    dynamic_reconfigure
    geometry_msgs
    move_base_msgs
    roscpp
    std_msgs
    teb_local_planner
    tf2
    tf2_geometry_msgs
    tf2_ros
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${YAML_CPP_INCLUDE_DIRS}
)

add_library(road_geometry
  src/road_geometry.cpp
)
target_link_libraries(road_geometry
  ${catkin_LIBRARIES}
)

add_library(road_yield_config
  src/config_loader.cpp
)
add_dependencies(road_yield_config
  ${catkin_EXPORTED_TARGETS}
)
target_link_libraries(road_yield_config
  ${catkin_LIBRARIES}
  yaml-cpp
)

add_library(road_yield_core
  src/navigation_client.cpp
  src/pose_provider.cpp
  src/road_yield_manager.cpp
  src/stair_controller.cpp
  src/vehicle_monitor.cpp
)
add_dependencies(road_yield_core
  ${catkin_EXPORTED_TARGETS}
)
target_link_libraries(road_yield_core
  road_geometry
  road_yield_config
  ${catkin_LIBRARIES}
)

add_executable(road_yield_node
  src/road_yield_node.cpp
)
add_dependencies(road_yield_node
  ${catkin_EXPORTED_TARGETS}
)
target_link_libraries(road_yield_node
  road_yield_core
  ${catkin_LIBRARIES}
)

if(CATKIN_ENABLE_TESTING)
  catkin_add_gtest(test_road_geometry test/test_main.cpp test/test_road_geometry.cpp)
  if(TARGET test_road_geometry)
    target_link_libraries(test_road_geometry road_geometry ${catkin_LIBRARIES})
  endif()

  catkin_add_gtest(test_config_loader test/test_main.cpp test/test_config_loader.cpp)
  if(TARGET test_config_loader)
    target_compile_definitions(test_config_loader PRIVATE
      TEST_CONFIG_PATH="${CMAKE_CURRENT_SOURCE_DIR}/config/path_a2/road_yield.yaml"
      TEST_STAIR_CONFIG_PATH="${CMAKE_CURRENT_SOURCE_DIR}/test/data/stair_config.yaml"
      TEST_PATROL_ONLY_CONFIG_PATH="${CMAKE_CURRENT_SOURCE_DIR}/test/data/patrol_only_config.yaml"
      TEST_MISSING_YIELD_CONFIG_PATH="${CMAKE_CURRENT_SOURCE_DIR}/test/data/missing_yield_config.yaml"
      TEST_PARTIAL_YIELD_CONFIG_PATH="${CMAKE_CURRENT_SOURCE_DIR}/test/data/partial_yield_config.yaml"
    )
    target_link_libraries(test_config_loader road_yield_config road_geometry ${catkin_LIBRARIES})
  endif()

  catkin_add_gtest(test_detection_window test/test_main.cpp test/test_detection_window.cpp)

  catkin_add_gtest(test_avoidance_strategy test/test_main.cpp test/test_avoidance_strategy.cpp)
  if(TARGET test_avoidance_strategy)
    target_link_libraries(test_avoidance_strategy road_geometry ${catkin_LIBRARIES})
  endif()
endif()

install(TARGETS road_geometry road_yield_config road_yield_core road_yield_node
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
)
install(DIRECTORY config launch
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)
