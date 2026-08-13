#pragma once

#include "domain.h"
#include "geo.h"
#include <algorithm>
#include <cstdint>
#include <deque>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace database {
namespace transport_catalogue {

using namespace database::geo;
using namespace database::detail;

struct Stop {
  std::string name;
  Coordinates coords;
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

class TransportCatalogue {
public:
  explicit TransportCatalogue() = default;

  ~TransportCatalogue() = default;

  void AddBus(const std::string &name,
              const std::vector<std::string_view> &route_names,
              bool is_roundtrip);

  void AddStop(const std::string &name, Coordinates coords);

  void SetStopDistance(const Stop *from, const Stop *to, int64_t distance);

  const Bus *FindBus(std::string_view name) const;

  const Stop *FindStop(std::string_view name) const;

  std::optional<BusInfoResult> BusInfo(std::string_view name) const;

  std::optional<BusesByStop> StopInfo(std::string_view name) const;

  int64_t GetDistance(const Stop *from, const Stop *to) const;

  std::vector<const Bus *> GetAllBuses() const {
    std::vector<const Bus *> result;
    result.reserve(buses_deque_.size());
    for (const Bus &bus : buses_deque_) {
      result.push_back(&bus);
    }
    std::sort(result.begin(), result.end(),
              [](const Bus *a, const Bus *b) { return a->name < b->name; });
    return result;
  }

private:
  std::unordered_map<std::string_view, const Stop *, StringViewHash,
                     StringViewEqual>
      stops_{};
  std::unordered_map<std::string_view, const Bus *, StringViewHash,
                     StringViewEqual>
      buses_{};
  std::unordered_map<const Stop *, BusesByStop> buses_by_stops_{};
  std::unordered_map<StopsPair, int64_t, PairPtrHash> distance_between_stops_{};

  std::deque<Bus> buses_deque_{};
  std::deque<Stop> stops_deque_{};

  uint64_t FindUniqueStops(const Bus *bus) const {
    std::unordered_set<std::string_view, StringViewHash, StringViewEqual> tmp;
    for (auto stop_ptr : bus->route) {
      tmp.insert(stop_ptr->name);
    }
    return tmp.size();
  }

  void AddBusToStopIndex(const Bus &bus) {
    for (const Stop *stop : bus.route) {
      if (buses_by_stops_.find(stop) != buses_by_stops_.end()) {
        auto &buses = buses_by_stops_.at(stop);
        buses.insert(&bus);
      } else {
        buses_by_stops_.insert({stop, {&bus}});
      }
    }
  }

  BusesByStop FindBusesByStop(const Stop *stop) const {
    auto ptr = buses_by_stops_.find(stop);
    if (ptr != buses_by_stops_.end()) {
      return ptr->second;
    }
    return {};
  }

  double ComputeGeographicLength(const Bus *bus) const {
    double geo_length = 0.0;
    if (bus->route.empty()) {
      return 0.0;
    }

    bool is_first = true;
    Coordinates first;
    Coordinates second;

    for (const Stop *stop_ptr : bus->route) {
      if (is_first) {
        first = stop_ptr->coords;
        is_first = false;
        continue;
      }

      second = stop_ptr->coords;
      geo_length += ComputeDistance(first, second);
      first = second;
    }

    return geo_length;
  }

  double ComputeRoadLength(const Bus *bus) const {
    double road_length = 0.0;
    if (bus->route.empty()) {
      return 0.0;
    }

    bool is_first = true;
    const Stop *first;
    const Stop *second;

    for (const Stop *stop_ptr : bus->route) {
      if (is_first) {
        first = stop_ptr;
        is_first = false;
        continue;
      }

      second = stop_ptr;
      road_length += GetDistance(first, second);
      first = second;
    }

    return road_length;
  }
};

} // namespace transport_catalogue
} // namespace database
