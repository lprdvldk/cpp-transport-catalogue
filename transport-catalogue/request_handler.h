#pragma once

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
    explicit RequestHandler(const database::transport_catalogue::TransportCatalogue& db);

    std::optional<BusStat> GetBusStat(std::string_view bus_name) const;
    std::optional<StopStat> GetStopStat(std::string_view stop_name) const;

private:
    const database::transport_catalogue::TransportCatalogue& db_;
};
