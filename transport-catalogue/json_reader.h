#pragma once

#include "domain.h"
#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class JsonReader {
public:
    explicit JsonReader(database::transport_catalogue::TransportCatalogue& catalogue);

    void LoadBaseRequests(const json::Node& document);

    static renderer::RenderSettings ParseRenderSettings(const json::Node& document);

    json::Node ProcessStatRequests(const json::Node& document, const RequestHandler& handler) const;

private:
    database::transport_catalogue::TransportCatalogue& catalogue_;

    void ProcessBaseRequests(const json::Node::Array& base_requests);
    void AddStop(const json::Node::Object& stop_request);
    void AddStopDistances(const json::Node::Object& stop_request);
    void AddBus(const json::Node::Object& bus_request);
    static bool IsStopRequest(const json::Node& item);
    static bool IsBusRequest(const json::Node& item);
    static void AppendReturnTrip(std::vector<std::string_view>& route_names);

    json::Node::Array BuildStatResponses(const json::Node::Array& stat_requests,
                                         const RequestHandler& handler) const;
    static json::Node::Object BuildBusResponse(int id,
        const std::optional<database::BusInfoResult>& stat);
    static json::Node::Object BuildStopResponse(int id,
        const std::optional<std::reference_wrapper<const database::BusesByStop>>& stat);
    static json::Node::Object BuildMapResponse(int id, const std::string& map_svg);
    static std::string GetStringField(const json::Node::Object& obj, const std::string& key);

    static renderer::RenderSettings ParseRenderSettingsObject(const json::Node& settings_node);
    static svg::Color ParseColor(const json::Node& node);
    static double GetNumberField(const json::Node::Object& obj, const std::string& key, double def);
    static int GetIntField(const json::Node::Object& obj, const std::string& key, int def);
    static svg::Point GetPointField(const json::Node::Object& obj, const std::string& key);
};
