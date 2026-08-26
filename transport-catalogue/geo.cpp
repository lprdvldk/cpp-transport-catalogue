#define _USE_MATH_DEFINES
#include "geo.h"

#include <algorithm>
#include <cmath>

namespace database {
namespace geo {
double ComputeDistance(Coordinates from, Coordinates to) {
  const double dr = M_PI / 180.0;
  double cos_angle = std::sin(from.lat * dr) * std::sin(to.lat * dr) +
                     std::cos(from.lat * dr) * std::cos(to.lat * dr) *
                         std::cos(std::abs(from.lng - to.lng) * dr);
  cos_angle = std::clamp(cos_angle, -1.0, 1.0);
  return std::acos(cos_angle) * 6371000;
}
}  // namespace geo

}  // namespace database
