#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"

#include <optional>
#include <set>
#include <string>
#include <string_view>

struct BusStat {
    size_t stop_count;
    size_t unique_stop_count;
    double route_length;
    double curvature;
};

struct StopStat {
    std::set<std::string> buses;
};

class RequestHandler {
public:
    RequestHandler(const database::transport_catalogue::TransportCatalogue& db,
                   const renderer::MapRenderer& renderer);

    std::optional<BusStat> GetBusStat(std::string_view bus_name) const;
    std::optional<StopStat> GetStopStat(std::string_view stop_name) const;

    std::string RenderMap() const;

private:
    const database::transport_catalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};
