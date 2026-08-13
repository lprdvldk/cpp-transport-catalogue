#include "request_handler.h"

RequestHandler::RequestHandler(const database::transport_catalogue::TransportCatalogue& db)
    : db_(db) {}

std::optional<BusStat> RequestHandler::GetBusStat(std::string_view bus_name) const {
    auto info = db_.BusInfo(bus_name);
    if (!info) {
        return std::nullopt;
    }
    BusStat stat;
    stat.stop_count = info->stops_on_route;
    stat.unique_stop_count = info->unique_stops;
    stat.route_length = info->route_length;
    stat.curvature = info->curvature;
    return stat;
}

std::optional<StopStat> RequestHandler::GetStopStat(std::string_view stop_name) const {
    auto buses_set = db_.StopInfo(stop_name);
    if (!buses_set) {
        return std::nullopt;
    }
    StopStat stat;
    for (const auto* bus : *buses_set) {
        stat.buses.insert(bus->name);
    }
    return stat;
}
