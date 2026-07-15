cmake_minimum_required(VERSION 3.0.2)
project(dynamic_obstacles)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(catkin REQUIRED COMPONENTS
  costmap_converter
  geometry_msgs
  message_generation
  roscpp
  std_msgs
  tf2
  uuid_msgs
  visualization_msgs
)

add_message_files(
  FILES
    ObjectClassification.msg
    TrackedObject.msg
    TrackedObjectArray.msg
)

generate_messages(
  DEPENDENCIES
    geometry_msgs
    std_msgs
    uuid_msgs
)

catkin_package(
  CATKIN_DEPENDS
    costmap_converter
    geometry_msgs
    message_runtime
    roscpp
    std_msgs
    tf2
    uuid_msgs
    visualization_msgs
)

include_directories(
  ${catkin_INCLUDE_DIRS}
)

add_executable(teb_obstacle_adapter
  src/teb_obstacle_adapter.cpp
)
add_dependencies(teb_obstacle_adapter
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS}
)
target_link_libraries(teb_obstacle_adapter
  ${catkin_LIBRARIES}
)

install(TARGETS teb_obstacle_adapter
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(DIRECTORY launch
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)
