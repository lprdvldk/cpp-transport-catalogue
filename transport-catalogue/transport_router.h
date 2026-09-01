#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace routing {

struct RoutingSettings {
    int bus_wait_time = 0;     // minutes
    double bus_velocity = 0.0; // km/h
};

struct WaitItem {
    std::string stop_name;
    double time = 0.0;
};

struct BusItem {
    std::string bus_name;
    int span_count = 0;
    double time = 0.0;
};

using RouteItem = std::variant<WaitItem, BusItem>;

struct RouteInfo {
    double total_time = 0.0;
    std::vector<RouteItem> items;
};

class TransportRouter {
  public:
    TransportRouter(const database::transport_catalogue::TransportCatalogue& catalogue, RoutingSettings settings);

    TransportRouter(const TransportRouter&) = delete;
    TransportRouter& operator=(const TransportRouter&) = delete;
    TransportRouter(TransportRouter&&) = delete;
    TransportRouter& operator=(TransportRouter&&) = delete;

    std::optional<RouteInfo> BuildRoute(std::string_view from, std::string_view to) const;

  private:
    using Stop = database::Stop;
    using Bus = database::Bus;
    using TransportCatalogue = database::transport_catalogue::TransportCatalogue;

    struct EdgeInfo {
        RouteItem item;
    };

    struct BuildData {
        graph::DirectedWeightedGraph<double> graph;
        std::vector<EdgeInfo> edge_infos;
        std::unordered_map<const Stop*, graph::VertexId> stop_wait_vertex;
    };

    static BuildData BuildGraph(const TransportCatalogue& catalogue, RoutingSettings settings);

    const TransportCatalogue& catalogue_;
    BuildData build_;
    graph::Router<double> router_;
};

} // namespace routing
