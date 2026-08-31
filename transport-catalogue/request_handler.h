#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>

#include "domain.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

class RequestHandler {
  public:
    RequestHandler(const database::transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer,
                   const routing::TransportRouter& router);

    std::optional<database::BusInfoResult> GetBusStat(std::string_view bus_name) const;
    std::optional<std::reference_wrapper<const database::BusesByStop>> GetStopStat(std::string_view stop_name) const;
    std::optional<routing::RouteInfo> GetRouteInfo(std::string_view from, std::string_view to) const;

    std::string RenderMap() const;

  private:
    const database::transport_catalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    const routing::TransportRouter& router_;
};