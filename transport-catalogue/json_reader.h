#pragma once

#include <string_view>

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

class JsonReader {
 public:
  explicit JsonReader(
      database::transport_catalogue::TransportCatalogue& catalogue);

  void LoadBaseRequests(const json::Document& doc);

  static renderer::RenderSettings ParseRenderSettings(
      const json::Document& doc);

  json::Document ProcessStatRequests(const json::Document& doc,
                                     const RequestHandler& handler) const;

 private:
  database::transport_catalogue::TransportCatalogue& catalogue_;

  void LoadStops(const json::Array& base_requests);
  void LoadStopDistances(const json::Array& base_requests);
  void LoadBuses(const json::Array& base_requests);

  static svg::Color ParseColor(const json::Node& node);

  json::Node BuildResponse(const json::Dict& request,
                           const RequestHandler& handler) const;
  json::Node BuildBusResponse(int request_id, std::string_view name,
                              const RequestHandler& handler) const;
  json::Node BuildStopResponse(int request_id, std::string_view name,
                               const RequestHandler& handler) const;
  json::Node BuildMapResponse(int request_id,
                              const RequestHandler& handler) const;
  static json::Node BuildErrorResponse(int request_id);
};
