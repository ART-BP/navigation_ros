#ifndef ROAD_YIELD_ROAD_GEOMETRY_H_
#define ROAD_YIELD_ROAD_GEOMETRY_H_

#include <cstddef>
#include <limits>
#include <vector>

namespace road_yield
{

struct Point2D
{
  double x{0.0};
  double y{0.0};
};

struct RoadCrossSection
{
  Point2D left;
  Point2D right;
};

struct RoadProjection
{
  bool valid{false};
  double station{0.0};
  double distance{std::numeric_limits<double>::infinity()};
  std::size_t segment_index{0};
};

// Consecutive left/right pairs form one quadrilateral of the detection region.
// Their order defines road connectivity, but not the robot's travel direction.
class RoadGeometry
{
public:
  explicit RoadGeometry(const std::vector<RoadCrossSection>& cross_sections);

  bool contains(const Point2D& point, RoadProjection* projection = nullptr) const;
  RoadProjection projectToCenterline(const Point2D& point) const;
  bool isAheadTowardGoal(const Point2D& point,
                         const Point2D& robot_position,
                         const Point2D& navigation_goal,
                         double ahead_margin,
                         double max_ahead_distance,
                         RoadProjection* projection = nullptr) const;

  const std::vector<RoadCrossSection>& crossSections() const;
  double length() const;

private:
  std::vector<RoadCrossSection> cross_sections_;
  std::vector<Point2D> centers_;
  std::vector<double> cumulative_lengths_;
};

double distance(const Point2D& lhs, const Point2D& rhs);

}  // namespace road_yield

#endif  // ROAD_YIELD_ROAD_GEOMETRY_H_
