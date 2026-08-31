#include "transport_router.h"

namespace routing {

TransportRouter::TransportRouter(const TransportCatalogue& catalogue, RoutingSettings settings)
    : catalogue_(catalogue), build_(BuildGraph(catalogue, settings)), router_(build_.graph) {
}

TransportRouter::BuildData TransportRouter::BuildGraph(const TransportCatalogue& catalogue,
                                                       const RoutingSettings& settings) {
    BuildData data;
    const auto& stops = catalogue.GetAllStops();

    data.graph = graph::DirectedWeightedGraph<double>(stops.size() * 2);

    graph::VertexId next_vertex = 0;
    for (const Stop& stop : stops) {
        const graph::VertexId wait_start = next_vertex;
        const graph::VertexId board = next_vertex + 1;
        next_vertex += 2;

        data.stop_wait_vertex[&stop] = wait_start;

        data.graph.AddEdge({wait_start, board, static_cast<double>(settings.bus_wait_time)});
        data.edge_infos.push_back(
            EdgeInfo{RouteItem{WaitItem{stop.name, static_cast<double>(settings.bus_wait_time)}}});
    }

    const double meters_per_minute = settings.bus_velocity * 1000.0 / 60.0;

    for (const Bus& bus : catalogue.GetAllBuses()) {
        const auto& route = bus.route;
        const size_t stop_count = route.size();

        for (size_t start = 0; start + 1 < stop_count; ++start) {
            const graph::VertexId board_vertex = data.stop_wait_vertex.at(route[start]) + 1;
            double cumulative_distance = 0.0;

            for (size_t end = start + 1; end < stop_count; ++end) {
                cumulative_distance += static_cast<double>(catalogue.GetDistance(route[end - 1], route[end]));
                const graph::VertexId wait_vertex = data.stop_wait_vertex.at(route[end]);
                const double time = cumulative_distance / meters_per_minute;

                data.graph.AddEdge({board_vertex, wait_vertex, time});
                data.edge_infos.push_back(EdgeInfo{RouteItem{BusItem{bus.name, static_cast<int>(end - start), time}}});
            }
        }
    }

    return data;
}

std::optional<RouteInfo> TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
    const Stop* from_stop = catalogue_.FindStop(from);
    const Stop* to_stop = catalogue_.FindStop(to);
    if (!from_stop || !to_stop) {
        return std::nullopt;
    }

    const auto from_it = build_.stop_wait_vertex.find(from_stop);
    const auto to_it = build_.stop_wait_vertex.find(to_stop);
    if (from_it == build_.stop_wait_vertex.end() || to_it == build_.stop_wait_vertex.end()) {
        return std::nullopt;
    }

    const auto route = router_.BuildRoute(from_it->second, to_it->second);
    if (!route) {
        return std::nullopt;
    }

    RouteInfo result;
    result.total_time = route->weight;
    result.items.reserve(route->edges.size());
    for (graph::EdgeId edge_id : route->edges) {
        result.items.push_back(build_.edge_infos[edge_id].item);
    }
    return result;
}

} // namespace routing