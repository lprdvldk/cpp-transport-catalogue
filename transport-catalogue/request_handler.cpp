#include "request_handler.h"

#include <algorithm>
#include <sstream>
#include <vector>

RequestHandler::RequestHandler(const database::transport_catalogue::TransportCatalogue& db,
                               const renderer::MapRenderer& renderer, const routing::TransportRouter& router)
    : db_(db), renderer_(renderer), router_(router) {
}

std::optional<database::BusInfoResult> RequestHandler::GetBusStat(std::string_view bus_name) const {
    return db_.BusInfo(bus_name);
}

std::optional<std::reference_wrapper<const database::BusesByStop>>
RequestHandler::GetStopStat(std::string_view stop_name) const {
    return db_.StopInfo(stop_name);
}

std::optional<routing::RouteInfo> RequestHandler::GetRouteInfo(std::string_view from, std::string_view to) const {
    return router_.BuildRoute(from, to);
}

std::string RequestHandler::RenderMap() const {
    using database::transport_catalogue::Bus;

    std::vector<const Bus*> sorted_buses;
    for (const Bus& bus : db_.GetAllBuses()) {
        sorted_buses.push_back(&bus);
    }
    std::sort(sorted_buses.begin(), sorted_buses.end(), [](const Bus* a, const Bus* b) {
        return a->name < b->name;
    });

    svg::Document map = renderer_.RenderRouteMap(sorted_buses);
    std::ostringstream out;
    map.Render(out);
    return out.str();
}