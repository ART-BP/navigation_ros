#include <road_yield/config_loader.h>

#include <dynamic_obstacles/ObjectClassification.h>
#include <ros/ros.h>
#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace road_yield
{
namespace
{

constexpr double kPi = 3.14159265358979323846;

std::string trim(const std::string& input)
{
  const std::size_t first = input.find_first_not_of(" \t\r\n");
  if (first == std::string::npos)
  {
    return std::string();
  }
  const std::size_t last = input.find_last_not_of(" \t\r\n");
  return input.substr(first, last - first + 1);
}

std::string removeUtf8Bom(const std::string& input)
{
  if (input.size() >= 3 && static_cast<unsigned char>(input[0]) == 0xef &&
      static_cast<unsigned char>(input[1]) == 0xbb && static_cast<unsigned char>(input[2]) == 0xbf)
  {
    return input.substr(3);
  }
  return input;
}

std::vector<std::string> splitCsvLine(const std::string& line)
{
  std::vector<std::string> values;
  std::string current;
  bool in_quotes = false;
  for (std::size_t i = 0; i < line.size(); ++i)
  {
    const char character = line[i];
    if (character == '"')
    {
      if (in_quotes && i + 1 < line.size() && line[i + 1] == '"')
      {
        current.push_back('"');
        ++i;
      }
      else
      {
        in_quotes = !in_quotes;
      }
    }
    else if (character == ',' && !in_quotes)
    {
      values.push_back(trim(current));
      current.clear();
    }
    else
    {
      current.push_back(character);
    }
  }
  if (in_quotes)
  {
    throw std::runtime_error("unterminated quoted CSV field");
  }
  values.push_back(trim(current));
  return values;
}

double parseFiniteDouble(const std::string& text, const std::string& context)
{
  std::size_t parsed = 0;
  const double value = std::stod(text, &parsed);
  if (parsed != text.size() || !std::isfinite(value))
  {
    throw std::runtime_error("invalid numeric value for " + context + ": " + text);
  }
  return value;
}

int parseInteger(const std::string& text, const std::string& context)
{
  std::size_t parsed = 0;
  const int value = std::stoi(text, &parsed);
  if (parsed != text.size())
  {
    throw std::runtime_error("invalid integer value for " + context + ": " + text);
  }
  return value;
}

std::string directoryName(const std::string& path)
{
  const std::size_t separator = path.find_last_of('/');
  if (separator == std::string::npos)
  {
    return ".";
  }
  return separator == 0 ? "/" : path.substr(0, separator);
}

std::string resolvePath(const std::string& yaml_path, const std::string& configured_path)
{
  if (configured_path.empty() || configured_path.front() == '/')
  {
    return configured_path;
  }
  return directoryName(yaml_path) + "/" + configured_path;
}

bool canOpenFile(const std::string& path)
{
  if (path.empty())
  {
    return false;
  }
  std::ifstream stream(path);
  return stream.good();
}

std::string normalizeFrame(const std::string& frame)
{
  return !frame.empty() && frame.front() == '/' ? frame.substr(1) : frame;
}

bool isStairCommand(const std::string& command)
{
  std::string normalized = trim(command);
  std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                 [](unsigned char character) {
                   return static_cast<char>(std::tolower(character));
                 });
  return normalized == "stair" || normalized == "stairs";
}

template <typename T>
T yamlValue(const YAML::Node& node, const std::string& key, const T& default_value)
{
  return node[key] ? node[key].as<T>() : default_value;
}

class CsvTable
{
public:
  explicit CsvTable(const std::string& path)
    : path_(path)
  {
    std::ifstream stream(path_);
    if (!stream)
    {
      throw std::runtime_error("could not open CSV: " + path_);
    }

    std::string line;
    if (!std::getline(stream, line))
    {
      throw std::runtime_error("CSV is empty: " + path_);
    }
    std::vector<std::string> header = splitCsvLine(line);
    if (!header.empty())
    {
      header[0] = removeUtf8Bom(header[0]);
    }
    for (std::size_t i = 0; i < header.size(); ++i)
    {
      if (header[i].empty() || columns_.count(header[i]))
      {
        throw std::runtime_error("empty or duplicate CSV column in " + path_);
      }
      columns_[header[i]] = i;
    }

    std::size_t file_line = 1;
    while (std::getline(stream, line))
    {
      ++file_line;
      if (trim(line).empty())
      {
        continue;
      }
      std::vector<std::string> row = splitCsvLine(line);
      if (row.size() > header.size())
      {
        throw std::runtime_error("too many values at " + path_ + ":" + std::to_string(file_line));
      }
      row.resize(header.size());
      rows_.push_back(std::move(row));
      file_lines_.push_back(file_line);
    }
  }

  void require(const std::vector<std::string>& names) const
  {
    for (const std::string& name : names)
    {
      if (!columns_.count(name))
      {
        throw std::runtime_error("CSV " + path_ + " is missing required column: " + name);
      }
    }
  }

  bool has(const std::string& name) const
  {
    return columns_.count(name) != 0;
  }

  const std::string& value(std::size_t row, const std::string& column) const
  {
    const std::string& result = rows_.at(row).at(columns_.at(column));
    if (result.empty())
    {
      throw std::runtime_error("empty " + column + " at " + path_ + ":" +
                               std::to_string(file_lines_.at(row)));
    }
    return result;
  }

  std::string optionalValue(std::size_t row, const std::string& column) const
  {
    return has(column) ? rows_.at(row).at(columns_.at(column)) : std::string();
  }

  std::size_t size() const
  {
    return rows_.size();
  }

private:
  std::string path_;
  std::map<std::string, std::size_t> columns_;
  std::vector<std::vector<std::string>> rows_;
  std::vector<std::size_t> file_lines_;
};

std::vector<NavigationPose> loadPoseCsv(const std::string& path,
                                        const std::string& name_prefix,
                                        bool yaw_is_degrees)
{
  CsvTable table(path);
  const std::string x_column = table.has("mx") ? "mx" : "x";
  const std::string y_column = table.has("my") ? "my" : "y";
  table.require({x_column, y_column, "yaw"});

  std::vector<NavigationPose> poses;
  poses.reserve(table.size());
  for (std::size_t row = 0; row < table.size(); ++row)
  {
    NavigationPose pose;
    pose.name = name_prefix + std::to_string(row + 1);
    const std::string configured_name = table.optionalValue(row, "name");
    if (!configured_name.empty())
    {
      pose.name = configured_name;
    }
    pose.position.x = parseFiniteDouble(table.value(row, x_column), x_column);
    pose.position.y = parseFiniteDouble(table.value(row, y_column), y_column);
    pose.yaw = parseFiniteDouble(table.value(row, "yaw"), "yaw");
    if (yaw_is_degrees)
    {
      pose.yaw *= kPi / 180.0;
    }
    pose.stair = isStairCommand(table.optionalValue(row, "cmd"));
    poses.push_back(pose);
  }
  if (poses.empty())
  {
    throw std::runtime_error("CSV contains no poses: " + path);
  }
  return poses;
}

void markStairRouteIndices(const YAML::Node& root,
                           const std::string& key,
                           int index_base,
                           std::vector<NavigationPose>& route)
{
  const YAML::Node indices = root[key];
  if (!indices)
  {
    return;
  }
  if (!indices.IsSequence())
  {
    throw std::runtime_error(key + " must be a sequence of waypoint indices");
  }

  for (std::size_t i = 0; i < indices.size(); ++i)
  {
    const int configured_index = indices[i].as<int>();
    const long long route_index =
        static_cast<long long>(configured_index) - static_cast<long long>(index_base);
    if (route_index < 0 || route_index >= static_cast<long long>(route.size()))
    {
      throw std::runtime_error(key + " contains out-of-range waypoint index: " +
                               std::to_string(configured_index));
    }
    route[static_cast<std::size_t>(route_index)].stair = true;
  }
}

void markConfiguredStairWaypoints(const YAML::Node& root,
                                  std::vector<NavigationPose>& route)
{
  const int index_base = yamlValue<int>(root, "stair_index_base", 1);
  markStairRouteIndices(root, "stair_indices", index_base, route);
  markStairRouteIndices(root, "stair_waypoint_indices", index_base, route);
}

void loadBoundaryCsv(const std::string& path,
                     std::vector<RoadCrossSection>& sections,
                     std::size_t& route_begin,
                     std::size_t& route_end)
{
  CsvTable table(path);
  table.require({"waypoint_line", "left_x", "left_y", "right_x", "right_y"});
  if (table.size() < 2)
  {
    throw std::runtime_error("road boundary CSV needs at least two rows: " + path);
  }

  int previous_line = -1;
  route_begin = std::numeric_limits<std::size_t>::max();
  route_end = 0;
  sections.reserve(table.size());

  for (std::size_t row = 0; row < table.size(); ++row)
  {
    const int waypoint_line = parseInteger(table.value(row, "waypoint_line"), "waypoint_line");
    if (waypoint_line < 2 || waypoint_line <= previous_line)
    {
      throw std::runtime_error("waypoint_line must be strictly increasing and include the CSV header line");
    }
    previous_line = waypoint_line;

    // waypoint_line is the physical patrol CSV line number. Line 1 is the
    // header, so patrol route index = waypoint_line - 2.
    const std::size_t route_index = static_cast<std::size_t>(waypoint_line - 2);
    route_begin = std::min(route_begin, route_index);
    route_end = std::max(route_end, route_index);

    RoadCrossSection section;
    section.left.x = parseFiniteDouble(table.value(row, "left_x"), "left_x");
    section.left.y = parseFiniteDouble(table.value(row, "left_y"), "left_y");
    section.right.x = parseFiniteDouble(table.value(row, "right_x"), "right_x");
    section.right.y = parseFiniteDouble(table.value(row, "right_y"), "right_y");
    sections.push_back(section);
  }
}

void loadTebOverrides(const YAML::Node& values, TebOverrides& overrides)
{
  if (!values)
  {
    return;
  }
  if (!values.IsMap())
  {
    throw std::runtime_error("stair_teb_params must be a map");
  }

  const std::set<std::string> supported = {
      "max_vel_x", "max_vel_theta", "acc_lim_x", "acc_lim_theta",
      "weight_kinematics_forward_drive", "weight_optimaltime", "weight_viapoint"};
  for (auto iterator = values.begin(); iterator != values.end(); ++iterator)
  {
    const std::string key = iterator->first.as<std::string>();
    if (!supported.count(key))
    {
      throw std::runtime_error("unsupported stair_teb_params key: " + key);
    }
  }

  if (values["max_vel_x"])
  {
    overrides.has_max_vel_x = true;
    overrides.max_vel_x = values["max_vel_x"].as<double>();
  }
  if (values["max_vel_theta"])
  {
    overrides.has_max_vel_theta = true;
    overrides.max_vel_theta = values["max_vel_theta"].as<double>();
  }
  if (values["acc_lim_x"])
  {
    overrides.has_acc_lim_x = true;
    overrides.acc_lim_x = values["acc_lim_x"].as<double>();
  }
  if (values["acc_lim_theta"])
  {
    overrides.has_acc_lim_theta = true;
    overrides.acc_lim_theta = values["acc_lim_theta"].as<double>();
  }
  if (values["weight_kinematics_forward_drive"])
  {
    overrides.has_weight_kinematics_forward_drive = true;
    overrides.weight_kinematics_forward_drive = values["weight_kinematics_forward_drive"].as<double>();
  }
  if (values["weight_optimaltime"])
  {
    overrides.has_weight_optimaltime = true;
    overrides.weight_optimaltime = values["weight_optimaltime"].as<double>();
  }
  if (values["weight_viapoint"])
  {
    overrides.has_weight_viapoint = true;
    overrides.weight_viapoint = values["weight_viapoint"].as<double>();
  }
}

}  // namespace

bool TebOverrides::empty() const
{
  return !has_max_vel_x && !has_max_vel_theta && !has_acc_lim_x && !has_acc_lim_theta &&
         !has_weight_kinematics_forward_drive && !has_weight_optimaltime && !has_weight_viapoint;
}

ManagerConfig loadManagerConfig(const std::string& path)
{
  const YAML::Node root = YAML::LoadFile(path);
  ManagerConfig config;

  config.planning_frame = normalizeFrame(yamlValue<std::string>(root, "frame_id", config.planning_frame));
  config.robot_base_frame = normalizeFrame(yamlValue<std::string>(root, "robot_base_frame", config.robot_base_frame));
  config.move_base_action = yamlValue<std::string>(root, "move_base_action", config.move_base_action);
  config.tracked_objects_topic =
      yamlValue<std::string>(root, "tracked_objects_topic", config.tracked_objects_topic);
  config.stair_mode_topic = yamlValue<std::string>(root, "stair_mode_topic", config.stair_mode_topic);
  config.teb_reconfigure_ns =
      yamlValue<std::string>(root, "teb_reconfigure_ns", config.teb_reconfigure_ns);

  config.wait_server_timeout = yamlValue<double>(root, "wait_server_timeout", config.wait_server_timeout);
  config.tf_timeout = yamlValue<double>(root, "tf_timeout", config.tf_timeout);
  config.control_rate = yamlValue<double>(root, "control_rate", config.control_rate);
  config.stair_publish_rate = yamlValue<double>(root, "stair_publish_rate", config.stair_publish_rate);
  config.stair_warmup_time = yamlValue<double>(root, "stair_warmup_time", config.stair_warmup_time);
  config.dynamic_reconfigure_timeout =
      yamlValue<double>(root, "dynamic_reconfigure_timeout", config.dynamic_reconfigure_timeout);
  config.route_goal_timeout = yamlValue<double>(root, "goal_timeout", config.route_goal_timeout);
  config.avoidance_goal_timeout =
      yamlValue<double>(root, "avoidance_goal_timeout", config.avoidance_goal_timeout);
  config.avoidance_progress_timeout = yamlValue<double>(
      root, "avoidance_progress_timeout", config.avoidance_progress_timeout);
  config.avoidance_min_progress =
      yamlValue<double>(root, "avoidance_min_progress", config.avoidance_min_progress);
  config.avoidance_candidate_limit =
      yamlValue<int>(root, "avoidance_candidate_limit", config.avoidance_candidate_limit);
  config.near_goal_advance_distance =
      yamlValue<double>(root, "near_goal_advance_distance", config.near_goal_advance_distance);
  config.failure_retry_distance =
      yamlValue<double>(root, "failure_retry_distance", config.failure_retry_distance);
  config.failure_retry_count = yamlValue<int>(root, "failure_retry_count", config.failure_retry_count);
  config.continue_on_route_failure =
      yamlValue<bool>(root, "continue_on_route_failure", config.continue_on_route_failure);

  config.perception_timeout = yamlValue<double>(root, "perception_timeout", config.perception_timeout);
  config.perception_warning_interval = yamlValue<double>(
      root, "perception_warning_interval", config.perception_warning_interval);
  // clear_wait_time is accepted as a compatibility alias. The timer now starts
  // when the shelter goal succeeds and is never reset by later detections.
  config.avoidance_wait_time = yamlValue<double>(
      root, "avoidance_wait_time", yamlValue<double>(root, "clear_wait_time", config.avoidance_wait_time));
  config.max_coasted_time = yamlValue<double>(root, "max_coasted_time", config.max_coasted_time);
  config.ahead_margin = yamlValue<double>(root, "ahead_margin", config.ahead_margin);
  config.max_detection_ahead =
      yamlValue<double>(root, "max_detection_ahead", config.max_detection_ahead);
  config.behind_margin = yamlValue<double>(root, "behind_margin", config.behind_margin);
  config.max_detection_behind =
      yamlValue<double>(root, "max_detection_behind", config.max_detection_behind);
  config.stationary_speed_threshold = yamlValue<double>(
      root, "stationary_speed_threshold", config.stationary_speed_threshold);
  config.opposing_speed_threshold = yamlValue<double>(
      root, "opposing_speed_threshold", config.opposing_speed_threshold);
  config.detection_window_size =
      yamlValue<int>(root, "detection_window_size", config.detection_window_size);
  config.detection_required_count =
      yamlValue<int>(root, "detection_required_count", config.detection_required_count);
  config.clear_required_count =
      yamlValue<int>(root, "clear_required_count", config.clear_required_count);
  config.require_perception_before_navigation = yamlValue<bool>(
      root, "require_perception_before_navigation", config.require_perception_before_navigation);

  if (!root["route_waypoint_file"])
  {
    throw std::runtime_error("configuration must specify route_waypoint_file");
  }
  const bool csv_yaw_degrees = yamlValue<bool>(root, "csv_yaw_degrees", true);
  const std::string configured_route_path = trim(root["route_waypoint_file"].as<std::string>());
  if (configured_route_path.empty())
  {
    throw std::runtime_error("route_waypoint_file must not be empty");
  }
  const std::string route_path = resolvePath(path, configured_route_path);

  config.route = loadPoseCsv(route_path, "route_", csv_yaw_degrees);
  markConfiguredStairWaypoints(root, config.route);

  const std::string configured_avoidance_path =
      trim(yamlValue<std::string>(root, "avoidance_waypoint_file", std::string()));
  const std::string configured_boundary_path =
      trim(yamlValue<std::string>(root, "road_boundary_file", std::string()));
  const std::string avoidance_path = resolvePath(path, configured_avoidance_path);
  const std::string boundary_path = resolvePath(path, configured_boundary_path);
  const bool avoidance_available = canOpenFile(avoidance_path);
  const bool boundary_available = canOpenFile(boundary_path);

  if (avoidance_available != boundary_available)
  {
    throw std::runtime_error(
        "road-yield configuration is incomplete: avoidance_waypoint_file and "
        "road_boundary_file must both be available or both be absent");
  }

  if (avoidance_available)
  {
    const std::vector<NavigationPose> shelters =
        loadPoseCsv(avoidance_path, "avoidance_", csv_yaw_degrees);
    config.avoidance_points.reserve(shelters.size());
    for (const NavigationPose& shelter : shelters)
    {
      AvoidancePoint point;
      point.shelter = shelter;
      config.avoidance_points.push_back(point);
    }
    loadBoundaryCsv(boundary_path, config.road_cross_sections,
                    config.detection_route_begin, config.detection_route_end);
    if (config.detection_route_end >= config.route.size())
    {
      throw std::runtime_error("boundary waypoint_line exceeds the patrol CSV line count");
    }
    config.road_yield_enabled = true;
  }
  else if (!configured_avoidance_path.empty() || !configured_boundary_path.empty())
  {
    ROS_WARN_STREAM("Neither road-yield CSV could be opened (avoidance='"
                    << avoidance_path << "', boundary='" << boundary_path
                    << "'); road-yield detection is disabled and patrol will continue");
  }
  else
  {
    ROS_INFO("No road-yield CSVs configured; running patrol-only mode");
  }

  const YAML::Node labels = root["vehicle_labels"];
  if (labels && labels.IsSequence())
  {
    for (std::size_t i = 0; i < labels.size(); ++i)
    {
      const int label = labels[i].as<int>();
      if (label < 0 || label > 255)
      {
        throw std::runtime_error("vehicle_labels values must be in [0, 255]");
      }
      config.vehicle_labels.insert(static_cast<uint8_t>(label));
    }
  }
  else
  {
    config.vehicle_labels = {
        dynamic_obstacles::ObjectClassification::CAR,
        dynamic_obstacles::ObjectClassification::TRUCK,
        dynamic_obstacles::ObjectClassification::BUS,
        dynamic_obstacles::ObjectClassification::TRAILER,
        dynamic_obstacles::ObjectClassification::MOTORCYCLE,
        dynamic_obstacles::ObjectClassification::BICYCLE,
        dynamic_obstacles::ObjectClassification::ELECTRIC_BICYCLE,
    };
  }

  loadTebOverrides(root["stair_teb_params"], config.stair_teb_params);

  if (config.control_rate <= 0.0 || config.stair_publish_rate <= 0.0 || config.perception_timeout <= 0.0 ||
      config.perception_warning_interval <= 0.0 ||
      config.avoidance_progress_timeout <= 0.0 || config.avoidance_min_progress <= 0.0 ||
      config.avoidance_candidate_limit <= 0 ||
      config.avoidance_wait_time < 0.0 || config.ahead_margin < 0.0 || config.behind_margin < 0.0 ||
      config.stationary_speed_threshold < 0.0 || config.opposing_speed_threshold < 0.0 ||
      config.detection_window_size <= 0 || config.detection_required_count <= 0 ||
      config.detection_required_count > config.detection_window_size || config.clear_required_count <= 0)
  {
    throw std::runtime_error("one or more timing/detection configuration values are outside their valid range");
  }

  ROS_INFO_STREAM("Loaded patrol=" << config.route.size() << ", avoidance="
                  << config.avoidance_points.size() << ", boundary_pairs="
                  << config.road_cross_sections.size() << ", road_yield_enabled="
                  << std::boolalpha << config.road_yield_enabled);
  if (config.road_yield_enabled)
  {
    ROS_INFO_STREAM("Road-yield detection patrol indexes="
                    << config.detection_route_begin + 1 << ".."
                    << config.detection_route_end + 1);
  }
  return config;
}

}  // namespace road_yield
