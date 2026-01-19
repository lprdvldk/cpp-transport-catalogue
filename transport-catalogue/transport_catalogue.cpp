#include "transport_catalogue.h"

namespace database::transport_catalogue {

void TransportCatalogue::AddBus(
    const std::string &name, const std::vector<std::string_view> &route_names) {
  std::vector<const Stop *> tmp;
  for (const std::string_view &name : route_names) {
    tmp.push_back(FindStop(name));
  }
  Bus bus{name, std::move(tmp)};

  buses_deque_.push_back(std::move(bus));
  const Bus &stored_bus = buses_deque_.back();

  buses_.emplace(stored_bus.name, &stored_bus);

  AddBusToStopIndex(stored_bus);
}

void TransportCatalogue::AddStop(const std::string &name, Coordinates coords) {
  Stop stop{name, coords};

  stops_deque_.push_back(std::move(stop));
  const Stop &stored_stop = stops_deque_.back();

  stops_.emplace(stored_stop.name, &stored_stop);
}

void TransportCatalogue::SetStopDistance(const Stop *from, const Stop *to,
                                         int64_t distance) {
  if (from == nullptr) {
    return;
  }

  if (to == nullptr) {
    return;
  }

  distance_between_stops_.insert({std::make_pair<const Stop *, const Stop *>(
                                      std::move(from), std::move(to)),
                                  distance});
}

std::optional<BusInfoResult>
TransportCatalogue::BusInfo(std::string_view name) const {
  const Bus *bus = FindBus(name);

  if (!bus) {
    return std::nullopt;
  }

  auto num_stops = bus->route.size();
  auto num_unique_stops = FindUniqueStops(bus);

  double geo_length = ComputeGeographicLength(bus);

  double road_length = ComputeRoadLength(bus);

  double curvature = geo_length > 0 ? road_length / geo_length : 0.0;

  return std::make_optional<BusInfoResult>(num_stops, num_unique_stops,
                                           road_length, curvature);
}
std::optional<BusesByStop>
TransportCatalogue::StopInfo(std::string_view name) const {
  const Stop *stop = FindStop(name);

  if (!stop) {
    return std::nullopt;
  }

  auto busses = FindBusesByStop(stop);

  if (busses.size() == 0) {
    return std::make_optional<BusesByStop>({});
  }

  return std::make_optional<BusesByStop>(std::move(busses));
}

const Bus *TransportCatalogue::FindBus(std::string_view name) const {
  auto it = buses_.find(name);
  if (it != buses_.end()) {
    return it->second;
  }
  return nullptr;
}

const Stop *TransportCatalogue::FindStop(std::string_view name) const {
  auto it = stops_.find(name);
  if (it != stops_.end()) {
    return it->second;
  }
  return nullptr;
}

int64_t TransportCatalogue::GetDistance(const Stop *from,
                                        const Stop *to) const {
  auto pair_ptr = distance_between_stops_.find(std::make_pair(from, to));

  if (pair_ptr == distance_between_stops_.end()) {
    auto pair_ptr = distance_between_stops_.find(std::make_pair(to, from));

    if (pair_ptr == distance_between_stops_.end()) {
      return 0;
    }

    return pair_ptr->second;
  }

  return pair_ptr->second;
}

} // namespace database::transport_catalogue