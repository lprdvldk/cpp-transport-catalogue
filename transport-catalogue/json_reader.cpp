#include "json_reader.h"

JsonReader::JsonReader(database::transport_catalogue::TransportCatalogue& catalogue,
                       const RequestHandler& handler)
    : catalogue_(catalogue), handler_(handler) {}

json::Node JsonReader::ProcessRequests(const json::Node& document) {
    if (!document.IsObject()) return json::Node();

    const auto& root = document.AsObject();

    auto base_it = root.find("base_requests");
    if (base_it != root.end() && base_it->second.IsArray()) {
        ProcessBaseRequests(base_it->second.AsArray());
    }

    auto stat_it = root.find("stat_requests");
    if (stat_it != root.end() && stat_it->second.IsArray()) {
        return json::Node(ProcessStatRequests(stat_it->second.AsArray()));
    }

    return json::Node();
}

void JsonReader::ProcessBaseRequests(const json::Node::Array& base_requests) {
    // Первый проход: добавляем все остановки (без расстояний)
    for (const auto& item : base_requests) {
        if (!item.IsObject()) continue;
        const auto& obj = item.AsObject();

        auto type_it = obj.find("type");
        if (type_it == obj.end() || !type_it->second.IsString()) continue;
        if (type_it->second.AsString() != "Stop") continue;

        auto name_it = obj.find("name");
        auto lat_it = obj.find("latitude");
        auto lng_it = obj.find("longitude");
        if (name_it == obj.end() || lat_it == obj.end() || lng_it == obj.end()) continue;

        std::string name = name_it->second.AsString();
        double lat = lat_it->second.AsNumber();
        double lng = lng_it->second.AsNumber();

        catalogue_.AddStop(name, {lat, lng});
    }

    // Второй проход: добавляем расстояния и маршруты
    for (const auto& item : base_requests) {
        if (!item.IsObject()) continue;
        const auto& obj = item.AsObject();

        auto type_it = obj.find("type");
        if (type_it == obj.end() || !type_it->second.IsString()) continue;

        if (type_it->second.AsString() == "Stop") {
            auto name_it = obj.find("name");
            auto road_it = obj.find("road_distances");
            if (name_it == obj.end() || road_it == obj.end()) continue;

            const auto* from_stop = catalogue_.FindStop(name_it->second.AsString());
            if (!from_stop) continue;

            if (road_it->second.IsObject()) {
                const auto& distances = road_it->second.AsObject();
                for (const auto& [to_name, dist_node] : distances) {
                    if (!dist_node.IsInt() && !dist_node.IsDouble()) continue;
                    int distance = dist_node.IsInt() ? dist_node.AsInt()
                                                     : static_cast<int>(dist_node.AsDouble());
                    const auto* to_stop = catalogue_.FindStop(to_name);
                    if (to_stop) {
                        catalogue_.SetStopDistance(from_stop, to_stop, distance);
                    }
                }
            }
        } else if (type_it->second.AsString() == "Bus") {
            auto name_it = obj.find("name");
            auto stops_it = obj.find("stops");
            auto roundtrip_it = obj.find("is_roundtrip");
            if (name_it == obj.end() || stops_it == obj.end() || roundtrip_it == obj.end()) continue;

            std::string bus_name = name_it->second.AsString();
            bool is_roundtrip = roundtrip_it->second.IsBool() ? roundtrip_it->second.AsBool() : false;

            if (!stops_it->second.IsArray()) continue;
            const auto& stops_array = stops_it->second.AsArray();

            std::vector<std::string_view> route_names;
            for (const auto& stop_node : stops_array) {
                if (!stop_node.IsString()) continue;
                route_names.push_back(stop_node.AsString());
            }
            if (route_names.empty()) continue;

            // Для некольцевого маршрута добавляем обратный путь (без дублирования последней остановки)
            if (!is_roundtrip) {
                size_t n = route_names.size();
                for (size_t i = n; i > 0; --i) {
                    if (i == n) continue; // пропускаем последний элемент, он уже есть
                    route_names.push_back(route_names[i - 1]);
                }
            }

            catalogue_.AddBus(bus_name, route_names);
        }
    }
}

json::Node::Array JsonReader::ProcessStatRequests(const json::Node::Array& stat_requests) {
    json::Node::Array results;

    for (const auto& request : stat_requests) {
        if (!request.IsObject()) continue;
        const auto& obj = request.AsObject();

        auto id_it = obj.find("id");
        auto type_it = obj.find("type");
        auto name_it = obj.find("name");
        if (id_it == obj.end() || type_it == obj.end() || name_it == obj.end()) continue;

        int id = id_it->second.IsInt() ? id_it->second.AsInt() : 0;
        std::string type = type_it->second.IsString() ? type_it->second.AsString() : "";
        std::string name = name_it->second.IsString() ? name_it->second.AsString() : "";

        json::Node::Object result;
        result["request_id"] = json::Node(id);

        if (type == "Bus") {
            auto bus_stat = handler_.GetBusStat(name);
            if (bus_stat) {
                result["stop_count"] = json::Node(static_cast<int>(bus_stat->stop_count));
                result["unique_stop_count"] = json::Node(static_cast<int>(bus_stat->unique_stop_count));
                result["route_length"] = json::Node(static_cast<int>(bus_stat->route_length));
                result["curvature"] = json::Node(bus_stat->curvature);
            } else {
                result["error_message"] = json::Node("not found");
            }
        } else if (type == "Stop") {
            auto stop_stat = handler_.GetStopStat(name);
            if (stop_stat) {
                json::Node::Array buses_array;
                for (const std::string& bus_name : stop_stat->buses) {
                    buses_array.push_back(json::Node(bus_name));
                }
                result["buses"] = json::Node(buses_array);
            } else {
                result["error_message"] = json::Node("not found");
            }
        }

        results.push_back(json::Node(result));
    }

    return results;
}
