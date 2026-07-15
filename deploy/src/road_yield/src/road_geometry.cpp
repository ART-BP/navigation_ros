#include <road_yield/road_geometry.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace road_yield
{
namespace
{

constexpr double kEpsilon = 1e-9;

double cross(const Point2D& a, const Point2D& b, const Point2D& p)
{
  return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

bool pointInTriangle(const Point2D& point, const Point2D& a, const Point2D& b, const Point2D& c)
{
  const double c1 = cross(a, b, point);
  const double c2 = cross(b, c, point);
  const double c3 = cross(c, a, point);
  const bool has_negative = c1 < -kEpsilon || c2 < -kEpsilon || c3 < -kEpsilon;
  const bool has_positive = c1 > kEpsilon || c2 > kEpsilon || c3 > kEpsilon;
  return !(has_negative && has_positive);
}

bool pointInRoadCell(const Point2D& point,
                     const RoadCrossSection& current,
                     const RoadCrossSection& next)
{
  // Split [left_i, left_i+1, right_i+1, right_i] into two triangles.
  return pointInTriangle(point, current.left, next.left, next.right) ||
         pointInTriangle(point, current.left, next.right, current.right);
}

RoadProjection projectToSegment(const Point2D& point,
                                const Point2D& start,
                                const Point2D& end,
                                double start_station,
                                std::size_t segment_index)
{
  const double dx = end.x - start.x;
  const double dy = end.y - start.y;
  const double length_squared = dx * dx + dy * dy;
  if (length_squared <= kEpsilon)
  {
    return RoadProjection{};
  }

  const double raw_t = ((point.x - start.x) * dx + (point.y - start.y) * dy) / length_squared;
  const double t = std::max(0.0, std::min(1.0, raw_t));
  const Point2D projected{start.x + t * dx, start.y + t * dy};

  RoadProjection result;
  result.valid = true;
  result.station = start_station + t * std::sqrt(length_squared);
  result.distance = distance(point, projected);
  result.segment_index = segment_index;
  return result;
}

}  // namespace

double distance(const Point2D& lhs, const Point2D& rhs)
{
  return std::hypot(lhs.x - rhs.x, lhs.y - rhs.y);
}

RoadGeometry::RoadGeometry(const std::vector<RoadCrossSection>& cross_sections)
  : cross_sections_(cross_sections)
{
  if (cross_sections_.empty())
  {
    return;
  }
  if (cross_sections_.size() < 2)
  {
    throw std::invalid_argument("road geometry needs at least two left/right cross sections");
  }

  centers_.reserve(cross_sections_.size());
  for (const RoadCrossSection& section : cross_sections_)
  {
    centers_.push_back(Point2D{0.5 * (section.left.x + section.right.x),
                               0.5 * (section.left.y + section.right.y)});
  }

  cumulative_lengths_.resize(centers_.size(), 0.0);
  for (std::size_t i = 1; i < centers_.size(); ++i)
  {
    const double segment_length = distance(centers_[i - 1], centers_[i]);
    if (segment_length <= kEpsilon)
    {
      throw std::invalid_argument("adjacent road cross-section centers must not coincide");
    }
    cumulative_lengths_[i] = cumulative_lengths_[i - 1] + segment_length;
  }
}

RoadProjection RoadGeometry::projectToCenterline(const Point2D& point) const
{
  RoadProjection best;
  for (std::size_t i = 0; i + 1 < centers_.size(); ++i)
  {
    const RoadProjection candidate =
        projectToSegment(point, centers_[i], centers_[i + 1], cumulative_lengths_[i], i);
    if (candidate.valid && candidate.distance < best.distance)
    {
      best = candidate;
    }
  }
  return best;
}

bool RoadGeometry::contains(const Point2D& point, RoadProjection* projection) const
{
  bool inside = false;
  RoadProjection best_inside_projection;

  for (std::size_t i = 0; i + 1 < cross_sections_.size(); ++i)
  {
    if (!pointInRoadCell(point, cross_sections_[i], cross_sections_[i + 1]))
    {
      continue;
    }

    inside = true;
    const RoadProjection candidate =
        projectToSegment(point, centers_[i], centers_[i + 1], cumulative_lengths_[i], i);
    if (candidate.valid && candidate.distance < best_inside_projection.distance)
    {
      best_inside_projection = candidate;
    }
    break;  // point can only be inside one road cell, so we can stop searching
  }

  if (projection != nullptr)
  {
    *projection = best_inside_projection;
  }
  return inside;
}

// Check if the point is ahead of the robot toward the navigation goal, within the specified margins.

bool RoadGeometry::isAheadTowardGoal(const Point2D& point,
                                     const Point2D& robot_position,
                                     const Point2D& navigation_goal,
                                     double ahead_margin,
                                     double max_ahead_distance,
                                     RoadProjection* projection) const
{
  RoadProjection vehicle_projection;
  if (!contains(point, &vehicle_projection) || !vehicle_projection.valid)
  {
    return false;
  }

  if (projection != nullptr)
  {
    *projection = vehicle_projection;
  }

  const double direction_x = navigation_goal.x - robot_position.x;
  const double direction_y = navigation_goal.y - robot_position.y;
  const double direction_length = std::hypot(direction_x, direction_y);
  if (direction_length <= kEpsilon)
  {
    return false;
  }

  const double vehicle_offset_x = point.x - robot_position.x;
  const double vehicle_offset_y = point.y - robot_position.y;
  const double forward_distance =
      (vehicle_offset_x * direction_x + vehicle_offset_y * direction_y) / direction_length;
  if (forward_distance <= ahead_margin)
  {
    return false;
  }
  return max_ahead_distance <= 0.0 || forward_distance <= max_ahead_distance;
}

const std::vector<RoadCrossSection>& RoadGeometry::crossSections() const
{
  return cross_sections_;
}

double RoadGeometry::length() const
{
  return cumulative_lengths_.empty() ? 0.0 : cumulative_lengths_.back();
}

}  // namespace road_yield
