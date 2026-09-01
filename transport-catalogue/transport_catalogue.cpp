#include "transport_catalogue.h"

namespace database {
namespace transport_catalogue {

void TransportCatalogue::AddBus(std::string name, const std::vector<std::string_view>& route_names, bool is_roundtrip) {
    if (buses_.find(name) != buses_.end()) {
        return;
    }

    std::vector<const Stop*> tmp;
    for (const std::string_view& stop_name : route_names) {
        const Stop* stop = FindStop(stop_name);
        if (stop) {
            tmp.push_back(stop);
        }
    }
    if (tmp.empty()) {
        return;
    }

    Bus bus{std::move(name), std::move(tmp), is_roundtrip};
    buses_deque_.push_back(std::move(bus));
    const Bus& stored_bus = buses_deque_.back();
    buses_.emplace(stored_bus.name, &stored_bus);
    AddBusToStopIndex(stored_bus);
}

void TransportCatalogue::AddStop(std::string name, Coordinates coords) {
    if (stops_.find(name) != stops_.end()) {
        return;
    }
    Stop stop{std::move(name), coords};
    stops_deque_.push_back(std::move(stop));
    const Stop& stored_stop = stops_deque_.back();
    stops_.emplace(stored_stop.name, &stored_stop);
}

void TransportCatalogue::SetStopDistance(const Stop* from, const Stop* to, int64_t distance) {
    if (from == nullptr || to == nullptr) {
        return;
    }
    distance_between_stops_.insert({std::make_pair(from, to), distance});
}

std::optional<BusInfoResult> TransportCatalogue::BusInfo(std::string_view name) const {
    const Bus* bus = FindBus(name);
    if (!bus) {
        return std::nullopt;
    }
    auto num_stops = bus->route.size();
    auto num_unique_stops = FindUniqueStops(bus);
    double geo_length = ComputeGeographicLength(bus);
    double road_length = ComputeRoadLength(bus);
    double curvature = geo_length > 0 ? road_length / geo_length : 0.0;
    return std::make_optional<BusInfoResult>(num_stops, num_unique_stops, road_length, curvature);
}

std::optional<std::reference_wrapper<const BusesByStop>> TransportCatalogue::StopInfo(std::string_view name) const {
    const Stop* stop = FindStop(name);
    if (!stop) {
        return std::nullopt;
    }
    return std::cref(FindBusesByStop(stop));
}

const Bus* TransportCatalogue::FindBus(std::string_view name) const {
    auto it = buses_.find(name);
    return (it != buses_.end()) ? it->second : nullptr;
}

const Stop* TransportCatalogue::FindStop(std::string_view name) const {
    auto it = stops_.find(name);
    return (it != stops_.end()) ? it->second : nullptr;
}

int64_t TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
    auto it = distance_between_stops_.find(std::make_pair(from, to));
    if (it != distance_between_stops_.end()) {
        return it->second;
    }
    auto it_rev = distance_between_stops_.find(std::make_pair(to, from));
    if (it_rev != distance_between_stops_.end()) {
        return it_rev->second;
    }
    return 0;
}

} // namespace transport_catalogue
} // namespace database
