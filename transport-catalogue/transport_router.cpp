#include "transport_router.h"

namespace routing {

static constexpr double kMetersPerMinutePerKmh = 1000.0 / 60.0;

TransportRouter::TransportRouter(const TransportCatalogue& catalogue, RoutingSettings settings)
    : catalogue_(catalogue), settings_(settings) {
    BuildGraph();
    router_ = std::make_unique<graph::Router<double>>(graph_);
}

void TransportRouter::BuildGraph() {
    graph_ = graph::DirectedWeightedGraph<double>(catalogue_.GetAllStops().size() * 2);

    AddStopVertices();
    AddBusEdges();
}

void TransportRouter::AddStopVertices() {
    graph::VertexId next_vertex = 0;
    for (const Stop& stop : catalogue_.GetAllStops()) {
        const graph::VertexId wait_start = next_vertex;
        const graph::VertexId board = next_vertex + 1;
        next_vertex += 2;

        stop_wait_vertex_[&stop] = wait_start;

        graph_.AddEdge({wait_start, board, static_cast<double>(settings_.bus_wait_time)});
        edge_infos_.push_back(EdgeInfo{RouteItem{WaitItem{stop.name, static_cast<double>(settings_.bus_wait_time)}}});
    }
}

void TransportRouter::AddBusEdges() {
    const double meters_per_minute = settings_.bus_velocity * kMetersPerMinutePerKmh;

    for (const Bus& bus : catalogue_.GetAllBuses()) {
        const auto& route = bus.route;
        const size_t stop_count = route.size();

        for (size_t start = 0; start + 1 < stop_count; ++start) {
            const graph::VertexId board_vertex = stop_wait_vertex_.at(route[start]) + 1;
            double cumulative_distance = 0.0;

            for (size_t end = start + 1; end < stop_count; ++end) {
                cumulative_distance += static_cast<double>(catalogue_.GetDistance(route[end - 1], route[end]));
                const graph::VertexId wait_vertex = stop_wait_vertex_.at(route[end]);
                const double time = cumulative_distance / meters_per_minute;

                graph_.AddEdge({board_vertex, wait_vertex, time});
                edge_infos_.push_back(EdgeInfo{RouteItem{BusItem{bus.name, static_cast<int>(end - start), time}}});
            }
        }
    }
}

std::optional<RouteInfo> TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
    const Stop* from_stop = catalogue_.FindStop(from);
    const Stop* to_stop = catalogue_.FindStop(to);
    if (!from_stop || !to_stop) {
        return std::nullopt;
    }

    const auto from_it = stop_wait_vertex_.find(from_stop);
    const auto to_it = stop_wait_vertex_.find(to_stop);
    if (from_it == stop_wait_vertex_.end() || to_it == stop_wait_vertex_.end()) {
        return std::nullopt;
    }

    const auto route = router_->BuildRoute(from_it->second, to_it->second);
    if (!route) {
        return std::nullopt;
    }

    RouteInfo result;
    result.total_time = route->weight;
    result.items.reserve(route->edges.size());
    for (graph::EdgeId edge_id : route->edges) {
        result.items.push_back(edge_infos_[edge_id].item);
    }
    return result;
}

} // namespace routing