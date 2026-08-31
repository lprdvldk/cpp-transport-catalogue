#pragma once

#include <cstdlib>
#include <vector>

#include "ranges.h"

namespace graph {

using VertexId = size_t;
using EdgeId = size_t;

template <typename Weight> struct Edge {
    VertexId from;
    VertexId to;
    Weight weight;
};

template <typename Weight> class DirectedWeightedGraph {
  private:
    using IncidenceList = std::vector<EdgeId>;
    using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

  public:
    DirectedWeightedGraph() = default;
    explicit DirectedWeightedGraph(size_t vertex_count) : incidence_lists_(vertex_count) {
    }

    EdgeId AddEdge(const Edge<Weight>& edge) {
        edges_.push_back(edge);
        const EdgeId id = edges_.size() - 1;
        incidence_lists_[edge.from].push_back(id);
        return id;
    }

    size_t GetVertexCount() const {
        return incidence_lists_.size();
    }

    size_t GetEdgeCount() const {
        return edges_.size();
    }

    const Edge<Weight>& GetEdge(EdgeId edge_id) const {
        return edges_[edge_id];
    }

    IncidentEdgesRange GetIncidentEdges(VertexId vertex) const {
        return ranges::AsRange(incidence_lists_[vertex]);
    }

  private:
    std::vector<Edge<Weight>> edges_;
    std::vector<IncidenceList> incidence_lists_;
};

} // namespace graph