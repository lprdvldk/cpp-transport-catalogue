#include "json_reader.h"

#include <algorithm>

JsonReader::JsonReader(database::transport_catalogue::TransportCatalogue& catalogue)
    : catalogue_(catalogue) {}


bool JsonReader::IsStopRequest(const json::Node& item) {
    if (!item.IsObject()) {
        return false;
    }
    const auto& obj = item.AsObject();
    auto type_it = obj.find("type");
    return type_it != obj.end() && type_it->second.IsString() &&
           type_it->second.AsString() == "Stop";
}

bool JsonReader::IsBusRequest(const json::Node& item) {
    if (!item.IsObject()) {
        return false;
    }
    const auto& obj = item.AsObject();
    auto type_it = obj.find("type");
    return type_it != obj.end() && type_it->second.IsString() &&
           type_it->second.AsString() == "Bus";
}

void JsonReader::AddStop(const json::Node::Object& stop_request) {
    auto name_it = stop_request.find("name");
    auto lat_it = stop_request.find("latitude");
    auto lng_it = stop_request.find("longitude");
    if (name_it == stop_request.end() || lat_it == stop_request.end() || lng_it == stop_request.end()) {
        return;
    }
    catalogue_.AddStop(name_it->second.AsString(),
                       {lat_it->second.AsNumber(), lng_it->second.AsNumber()});
}

void JsonReader::AddStopDistances(const json::Node::Object& stop_request) {
    auto name_it = stop_request.find("name");
    auto road_it = stop_request.find("road_distances");
    if (name_it == stop_request.end() || road_it == stop_request.end() || !road_it->second.IsObject()) {
        return;
    }

    const auto* from_stop = catalogue_.FindStop(name_it->second.AsString());
    if (!from_stop) {
        return;
    }

    for (const auto& [to_name, dist_node] : road_it->second.AsObject()) {
        if (!dist_node.IsInt() && !dist_node.IsDouble()) {
            continue;
        }
        const auto* to_stop = catalogue_.FindStop(to_name);
        if (!to_stop) {
            continue;
        }
        int distance = dist_node.IsInt() ? dist_node.AsInt() : static_cast<int>(dist_node.AsDouble());
        catalogue_.SetStopDistance(from_stop, to_stop, distance);
    }
}

void JsonReader::AppendReturnTrip(std::vector<std::string_view>& route_names) {
    const size_t n = route_names.size();
    for (size_t i = n; i > 1; --i) {
        route_names.push_back(route_names[i - 2]);
    }
}

void JsonReader::AddBus(const json::Node::Object& bus_request) {
    auto name_it = bus_request.find("name");
    auto stops_it = bus_request.find("stops");
    auto roundtrip_it = bus_request.find("is_roundtrip");
    if (name_it == bus_request.end() || stops_it == bus_request.end() ||
        roundtrip_it == bus_request.end() || !stops_it->second.IsArray()) {
        return;
    }

    bool is_roundtrip = roundtrip_it->second.IsBool() && roundtrip_it->second.AsBool();

    std::vector<std::string_view> route_names;
    for (const auto& stop_node : stops_it->second.AsArray()) {
        if (stop_node.IsString()) {
            route_names.push_back(stop_node.AsString());
        }
    }
    if (route_names.empty()) {
        return;
    }

    if (!is_roundtrip) {
        AppendReturnTrip(route_names);
    }

    catalogue_.AddBus(name_it->second.AsString(), route_names, is_roundtrip);
}

void JsonReader::ProcessBaseRequests(const json::Node::Array& base_requests) {
    for (const auto& item : base_requests) {
        if (IsStopRequest(item)) {
            AddStop(item.AsObject());
        }
    }

    for (const auto& item : base_requests) {
        if (IsStopRequest(item)) {
            AddStopDistances(item.AsObject());
        } else if (IsBusRequest(item)) {
            AddBus(item.AsObject());
        }
    }
}

void JsonReader::LoadBaseRequests(const json::Node& document) {
    if (!document.IsObject()) {
        return;
    }
    const auto& root = document.AsObject();
    auto base_it = root.find("base_requests");
    if (base_it != root.end() && base_it->second.IsArray()) {
        ProcessBaseRequests(base_it->second.AsArray());
    }
}

svg::Color JsonReader::ParseColor(const json::Node& node) {
    if (node.IsString()) {
        return svg::Color(node.AsString());
    }
    if (node.IsArray()) {
        const auto& arr = node.AsArray();
        if (arr.size() == 3) {
            return svg::Rgb(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt());
        } else if (arr.size() == 4) {
            return svg::Rgba(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt(), arr[3].AsNumber());
        }
    }
    return svg::NoneColor;
}

double JsonReader::GetNumberField(const json::Node::Object& obj, const std::string& key, double def) {
    auto it = obj.find(key);
    return (it != obj.end()) ? it->second.AsNumber() : def;
}

int JsonReader::GetIntField(const json::Node::Object& obj, const std::string& key, int def) {
    auto it = obj.find(key);
    return (it != obj.end() && it->second.IsInt()) ? it->second.AsInt() : def;
}

svg::Point JsonReader::GetPointField(const json::Node::Object& obj, const std::string& key) {
    auto it = obj.find(key);
    if (it != obj.end() && it->second.IsArray()) {
        const auto& arr = it->second.AsArray();
        if (arr.size() == 2) {
            return {arr[0].AsNumber(), arr[1].AsNumber()};
        }
    }
    return {};
}

renderer::RenderSettings JsonReader::ParseRenderSettingsObject(const json::Node& settings_node) {
    renderer::RenderSettings settings;
    if (!settings_node.IsObject()) {
        return settings;
    }
    const auto& obj = settings_node.AsObject();

    settings.width = GetNumberField(obj, "width", 0);
    settings.height = GetNumberField(obj, "height", 0);
    settings.padding = GetNumberField(obj, "padding", 0);
    settings.line_width = GetNumberField(obj, "line_width", 0);
    settings.stop_radius = GetNumberField(obj, "stop_radius", 0);
    settings.bus_label_font_size = GetIntField(obj, "bus_label_font_size", 0);
    settings.stop_label_font_size = GetIntField(obj, "stop_label_font_size", 0);
    settings.underlayer_width = GetNumberField(obj, "underlayer_width", 0);
    settings.bus_label_offset = GetPointField(obj, "bus_label_offset");
    settings.stop_label_offset = GetPointField(obj, "stop_label_offset");

    auto underlayer_it = obj.find("underlayer_color");
    if (underlayer_it != obj.end()) {
        settings.underlayer_color = ParseColor(underlayer_it->second);
    }

    auto palette_it = obj.find("color_palette");
    if (palette_it != obj.end() && palette_it->second.IsArray()) {
        for (const auto& color_node : palette_it->second.AsArray()) {
            settings.color_palette.push_back(ParseColor(color_node));
        }
    }

    return settings;
}

renderer::RenderSettings JsonReader::ParseRenderSettings(const json::Node& document) {
    if (!document.IsObject()) {
        return renderer::RenderSettings();
    }
    const auto& root = document.AsObject();
    auto it = root.find("render_settings");
    if (it == root.end()) {
        return renderer::RenderSettings();
    }
    return ParseRenderSettingsObject(it->second);
}

std::string JsonReader::GetStringField(const json::Node::Object& obj, const std::string& key) {
    auto it = obj.find(key);
    return (it != obj.end() && it->second.IsString()) ? it->second.AsString() : std::string();
}

json::Node::Object JsonReader::BuildBusResponse(int id,
    const std::optional<database::BusInfoResult>& stat) {
    json::Node::Object result;
    result["request_id"] = json::Node(id);
    if (stat) {
        result["stop_count"] = json::Node(static_cast<int>(stat->stops_on_route));
        result["unique_stop_count"] = json::Node(static_cast<int>(stat->unique_stops));
        result["route_length"] = json::Node(static_cast<int>(stat->route_length));
        result["curvature"] = json::Node(stat->curvature);
    } else {
        result["error_message"] = json::Node("not found");
    }
    return result;
}

json::Node::Object JsonReader::BuildStopResponse(int id,
    const std::optional<std::reference_wrapper<const database::BusesByStop>>& stat) {
    json::Node::Object result;
    result["request_id"] = json::Node(id);
    if (stat) {
        std::vector<std::string_view> bus_names;
        for (const database::Bus* bus : stat->get()) {
            bus_names.push_back(bus->name);
        }
        std::sort(bus_names.begin(), bus_names.end());

        json::Node::Array buses_array;
        for (std::string_view bus_name : bus_names) {
            buses_array.push_back(json::Node(std::string(bus_name)));
        }
        result["buses"] = json::Node(buses_array);
    } else {
        result["error_message"] = json::Node("not found");
    }
    return result;
}

json::Node::Object JsonReader::BuildMapResponse(int id, const std::string& map_svg) {
    json::Node::Object result;
    result["request_id"] = json::Node(id);
    result["map"] = json::Node(map_svg);
    return result;
}

json::Node::Array JsonReader::BuildStatResponses(const json::Node::Array& stat_requests,
                                                 const RequestHandler& handler) const {
    json::Node::Array results;

    for (const auto& request : stat_requests) {
        if (!request.IsObject()) {
            continue;
        }
        const auto& obj = request.AsObject();

        auto id_it = obj.find("id");
        auto type_it = obj.find("type");
        if (id_it == obj.end() || type_it == obj.end() || !type_it->second.IsString()) {
            continue;
        }

        int id = id_it->second.IsInt() ? id_it->second.AsInt() : 0;
        const std::string& type = type_it->second.AsString();

        if (type == "Bus") {
            std::string name = GetStringField(obj, "name");
            results.push_back(json::Node(BuildBusResponse(id, handler.GetBusStat(name))));
        } else if (type == "Stop") {
            std::string name = GetStringField(obj, "name");
            results.push_back(json::Node(BuildStopResponse(id, handler.GetStopStat(name))));
        } else if (type == "Map") {
            results.push_back(json::Node(BuildMapResponse(id, handler.RenderMap())));
        }
    }

    return results;
}

json::Node JsonReader::ProcessStatRequests(const json::Node& document, const RequestHandler& handler) const {
    if (!document.IsObject()) {
        return json::Node(json::Node::Array());
    }
    const auto& root = document.AsObject();
    auto stat_it = root.find("stat_requests");
    if (stat_it == root.end() || !stat_it->second.IsArray()) {
        return json::Node(json::Node::Array());
    }
    return json::Node(BuildStatResponses(stat_it->second.AsArray(), handler));
}
