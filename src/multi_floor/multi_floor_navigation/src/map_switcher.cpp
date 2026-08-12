#include "multi_floor_navigation/map_switcher.h"

#include <nav_msgs/LoadMap.h>
#include <std_srvs/Empty.h>

#include <exception>
#include <utility>

namespace multi_floor_navigation
{

MapSwitcher::MapSwitcher(ros::NodeHandle& node,
                         const TopologyGraph& graph,
                         std::string change_map_service,
                         std::string clear_costmaps_service)
  : graph_(graph)
  , change_map_client_(node.serviceClient<nav_msgs::LoadMap>(std::move(change_map_service)))
  , clear_costmaps_client_(node.serviceClient<std_srvs::Empty>(std::move(clear_costmaps_service)))
  , current_floor_(-1)
{
}

void MapSwitcher::set_current_floor(int floor_id)
{
  if (!graph_.has_floor(floor_id))
  {
    throw std::out_of_range("cannot set an unknown current floor: " + std::to_string(floor_id));
  }
  current_floor_ = floor_id;
}

int MapSwitcher::current_floor() const
{
  return current_floor_;
}

bool MapSwitcher::switch_to(int floor_id, std::string& message)
{
  if (current_floor_ == floor_id)
  {
    message = "requested floor map is already active";
    return true;
  }

  std::string map_path;
  try
  {
    map_path = graph_.floor_map_path(floor_id);
  }
  catch (const std::out_of_range&)
  {
    message = "target floor is not configured: " + std::to_string(floor_id);
    return false;
  }

  nav_msgs::LoadMap load_map;
  load_map.request.map_url = map_path;
  if (!change_map_client_.call(load_map))
  {
    message = "failed to call map_server change_map service";
    return false;
  }
  if (load_map.response.result != nav_msgs::LoadMapResponse::RESULT_SUCCESS)
  {
    message = "map_server rejected map '" + map_path + "' with result " +
              std::to_string(load_map.response.result);
    return false;
  }

  current_floor_ = floor_id;
  std_srvs::Empty clear_costmaps;
  if (!clear_costmaps_client_.call(clear_costmaps))
  {
    message = "map changed, but move_base clear_costmaps service failed";
    return false;
  }

  message = "map changed to floor " + std::to_string(floor_id);
  return true;
}

}  // namespace multi_floor_navigation
