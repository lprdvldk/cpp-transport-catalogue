#pragma once

#include "geo.h"

#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace database {
namespace detail {

struct StringViewHash {
  using is_transparent = void;

  size_t operator()(std::string_view sv) const noexcept {
    return std::hash<std::string_view>{}(sv);
  }
};

struct StringViewEqual {
  using is_transparent = void;

  bool operator()(std::string_view lhs, std::string_view rhs) const noexcept {
    return lhs == rhs;
  }
};

struct PairPtrHash {
  using is_transparent = void;

  template <typename T>
  size_t operator()(std::pair<const T *, const T *> ptrs_pair) const noexcept {
    auto h1 = std::hash<const void *>{}((ptrs_pair.first));
    auto h2 = std::hash<const void *>{}((ptrs_pair.second));

    return h1 ^ (h2 + 0x9e3779b97f4a7c15ull + (h1 << 6) + (h1 >> 2));
  }
};

} // namespace detail

struct Stop {
  std::string name;
  geo::Coordinates coords;
};

struct Bus {
  std::string name;
  std::vector<const Stop *> route;
  bool is_roundtrip = false;
};

struct BusInfoResult {
  size_t stops_on_route;
  size_t unique_stops;
  double route_length;
  double curvature;
};

using BusesByStop = std::unordered_set<const Bus *>;
using StopsPair = std::pair<const Stop *, const Stop *>;

} // namespace database
