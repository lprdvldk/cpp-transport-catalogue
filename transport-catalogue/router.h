#pragma once

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <vector>

#include "graph.h"

namespace graph {

template <typename Weight> class Router {
  private:
    using GraphType = DirectedWeightedGraph<Weight>;

  public:
    explicit Router(const GraphType& graph) : graph_(graph) {
        const size_t vertex_count = graph_.GetVertexCount();
        routes_internal_data_.assign(vertex_count, std::vector<std::optional<RouteInternalData>>(vertex_count));

        for (VertexId vertex = 0; vertex < vertex_count; ++vertex) {
            routes_internal_data_[vertex][vertex] = RouteInternalData{Weight{}, std::nullopt};
        }

        for (VertexId vertex_from = 0; vertex_from < vertex_count; ++vertex_from) {
            for (const EdgeId edge_id : graph_.GetIncidentEdges(vertex_from)) {
                const Edge<Weight>& edge = graph_.GetEdge(edge_id);
                auto& current = routes_internal_data_[edge.from][edge.to];
                if (!current || edge.weight < current->weight) {
                    current = RouteInternalData{edge.weight, edge_id};
                }
            }
        }

        for (VertexId vertex_through = 0; vertex_through < vertex_count; ++vertex_through) {
            RelaxRoutesThroughVertex(vertex_count, vertex_through);
        }
    }

    struct RouteInfo {
        Weight weight;
        std::vector<EdgeId> edges;
    };

    std::optional<RouteInfo> BuildRoute(VertexId from, VertexId to) const {
        const auto& route_data = routes_internal_data_[from][to];
        if (!route_data) {
            return std::nullopt;
        }

        std::vector<EdgeId> edges;
        std::optional<EdgeId> last_edge = route_data->prev_edge;
        VertexId current_to = to;
        while (last_edge) {
            edges.push_back(*last_edge);
            current_to = graph_.GetEdge(*last_edge).from;
            last_edge = routes_internal_data_[from][current_to]->prev_edge;
        }
        std::reverse(edges.begin(), edges.end());

        return RouteInfo{route_data->weight, std::move(edges)};
    }

  private:
    struct RouteInternalData {
        Weight weight;
        std::optional<EdgeId> prev_edge;
    };

    void RelaxRoutesThroughVertex(size_t vertex_count, VertexId vertex_through) {
        for (VertexId i = 0; i < vertex_count; ++i) {
            const auto& left = routes_internal_data_[i][vertex_through];
            if (!left) {
                continue;
            }
            for (VertexId j = 0; j < vertex_count; ++j) {
                const auto& right = routes_internal_data_[vertex_through][j];
                if (!right) {
                    continue;
                }
                const Weight candidate_weight = left->weight + right->weight;
                auto& current = routes_internal_data_[i][j];
                if (!current || candidate_weight < current->weight) {
                    current = RouteInternalData{candidate_weight, right->prev_edge};
                }
            }
        }
    }

    const GraphType& graph_;
    std::vector<std::vector<std::optional<RouteInternalData>>> routes_internal_data_;
};

} // namespace graph