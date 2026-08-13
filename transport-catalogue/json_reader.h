#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include <string>

class JsonReader {
public:
    JsonReader(database::transport_catalogue::TransportCatalogue& catalogue,
               const RequestHandler& handler);

    json::Node ProcessRequests(const json::Node& document);

    void LoadBaseRequests(const json::Node& document);

    renderer::RenderSettings ParseRenderSettingsFromDocument(const json::Node& document);
    renderer::RenderSettings ParseRenderSettings(const json::Node& node);
    static svg::Color ParseColor(const json::Node& node);

private:
    database::transport_catalogue::TransportCatalogue& catalogue_;
    const RequestHandler& handler_;

    void ProcessBaseRequests(const json::Node::Array& base_requests);
    json::Node::Array ProcessStatRequests(const json::Node::Array& stat_requests);
};
