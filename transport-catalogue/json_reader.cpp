#include "json_reader.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "json_builder.h"

using namespace std::literals;

JsonReader::JsonReader(
    database::transport_catalogue::TransportCatalogue& catalogue)
    : catalogue_(catalogue) {}

void JsonReader::LoadStops(const json::Array& base_requests) {
  for (const json::Node& node : base_requests) {
    const json::Dict& request = node.AsDict();
    if (request.at("type"s).AsString() != "Stop"s) {
      continue;
    }
    const std::string& name = request.at("name"s).AsString();
    database::geo::Coordinates coords{request.at("latitude"s).AsDouble(),
                                      request.at("longitude"s).AsDouble()};
    catalogue_.AddStop(name, coords);
  }
}

void JsonReader::LoadStopDistances(const json::Array& base_requests) {
  for (const json::Node& node : base_requests) {
    const json::Dict& request = node.AsDict();
    if (request.at("type"s).AsString() != "Stop"s) {
      continue;
    }

    const database::Stop* from =
        catalogue_.FindStop(request.at("name"s).AsString());

    auto distances_it = request.find("road_distances"s);
    if (distances_it == request.end()) {
      continue;
    }

    for (const auto& [stop_name, distance_node] :
         distances_it->second.AsDict()) {
      const database::Stop* to = catalogue_.FindStop(stop_name);
      catalogue_.SetStopDistance(from, to, distance_node.AsInt());
    }
  }
}

void JsonReader::LoadBuses(const json::Array& base_requests) {
  for (const json::Node& node : base_requests) {
    const json::Dict& request = node.AsDict();
    if (request.at("type"s).AsString() != "Bus"s) {
      continue;
    }

    const std::string& name = request.at("name"s).AsString();
    const bool is_roundtrip = request.at("is_roundtrip"s).AsBool();

    std::vector<std::string_view> stop_names;
    for (const json::Node& stop_node : request.at("stops"s).AsArray()) {
      stop_names.push_back(stop_node.AsString());
    }

    if (!is_roundtrip && stop_names.size() > 1) {
      const size_t one_way_size = stop_names.size();
      for (size_t i = one_way_size - 1; i-- > 0;) {
        stop_names.push_back(stop_names[i]);
      }
    }

    catalogue_.AddBus(name, stop_names, is_roundtrip);
  }
}

void JsonReader::LoadBaseRequests(const json::Document& doc) {
  const json::Dict& root = doc.GetRoot().AsDict();
  auto it = root.find("base_requests"s);
  if (it == root.end()) {
    return;
  }
  const json::Array& base_requests = it->second.AsArray();

  LoadStops(base_requests);
  LoadStopDistances(base_requests);
  LoadBuses(base_requests);
}

svg::Color JsonReader::ParseColor(const json::Node& node) {
  if (node.IsString()) {
    return node.AsString();
  }

  const json::Array& components = node.AsArray();
  if (components.size() == 3) {
    return svg::Rgb(components[0].AsInt(), components[1].AsInt(),
                    components[2].AsInt());
  }
  if (components.size() == 4) {
    return svg::Rgba(components[0].AsInt(), components[1].AsInt(),
                     components[2].AsInt(), components[3].AsDouble());
  }

  throw std::logic_error("Invalid color format in render_settings"s);
}

renderer::RenderSettings JsonReader::ParseRenderSettings(
    const json::Document& doc) {
  renderer::RenderSettings settings;

  const json::Dict& root = doc.GetRoot().AsDict();
  auto it = root.find("render_settings"s);
  if (it == root.end()) {
    return settings;
  }
  const json::Dict& rs = it->second.AsDict();

  settings.width = rs.at("width"s).AsDouble();
  settings.height = rs.at("height"s).AsDouble();
  settings.padding = rs.at("padding"s).AsDouble();
  settings.line_width = rs.at("line_width"s).AsDouble();
  settings.stop_radius = rs.at("stop_radius"s).AsDouble();

  settings.bus_label_font_size = rs.at("bus_label_font_size"s).AsInt();
  const json::Array& bus_offset = rs.at("bus_label_offset"s).AsArray();
  settings.bus_label_offset =
      svg::Point(bus_offset.at(0).AsDouble(), bus_offset.at(1).AsDouble());

  settings.stop_label_font_size = rs.at("stop_label_font_size"s).AsInt();
  const json::Array& stop_offset = rs.at("stop_label_offset"s).AsArray();
  settings.stop_label_offset =
      svg::Point(stop_offset.at(0).AsDouble(), stop_offset.at(1).AsDouble());

  settings.underlayer_color = ParseColor(rs.at("underlayer_color"s));
  settings.underlayer_width = rs.at("underlayer_width"s).AsDouble();

  for (const json::Node& color_node : rs.at("color_palette"s).AsArray()) {
    settings.color_palette.push_back(ParseColor(color_node));
  }

  return settings;
}

json::Node JsonReader::BuildErrorResponse(int request_id) {
  return json::Builder{}
      .StartDict()
      .Key("request_id"s)
      .Value(request_id)
      .Key("error_message"s)
      .Value("not found"s)
      .EndDict()
      .Build();
}

json::Node JsonReader::BuildBusResponse(int request_id, std::string_view name,
                                        const RequestHandler& handler) const {
  const auto stat = handler.GetBusStat(name);
  if (!stat) {
    return BuildErrorResponse(request_id);
  }

  return json::Builder{}
      .StartDict()
      .Key("request_id"s)
      .Value(request_id)
      .Key("curvature"s)
      .Value(stat->curvature)
      .Key("route_length"s)
      .Value(stat->route_length)
      .Key("stop_count"s)
      .Value(static_cast<int>(stat->stops_on_route))
      .Key("unique_stop_count"s)
      .Value(static_cast<int>(stat->unique_stops))
      .EndDict()
      .Build();
}

json::Node JsonReader::BuildStopResponse(int request_id, std::string_view name,
                                         const RequestHandler& handler) const {
  const auto stat = handler.GetStopStat(name);
  if (!stat) {
    return BuildErrorResponse(request_id);
  }

  std::vector<std::string_view> bus_names;
  for (const database::Bus* bus : stat->get()) {
    bus_names.push_back(bus->name);
  }
  std::sort(bus_names.begin(), bus_names.end());

  json::Array buses;
  buses.reserve(bus_names.size());
  for (std::string_view bus_name : bus_names) {
    buses.emplace_back(std::string(bus_name));
  }

  return json::Builder{}
      .StartDict()
      .Key("request_id"s)
      .Value(request_id)
      .Key("buses"s)
      .Value(std::move(buses))
      .EndDict()
      .Build();
}

json::Node JsonReader::BuildMapResponse(int request_id,
                                        const RequestHandler& handler) const {
  return json::Builder{}
      .StartDict()
      .Key("request_id"s)
      .Value(request_id)
      .Key("map"s)
      .Value(handler.RenderMap())
      .EndDict()
      .Build();
}

json::Node JsonReader::BuildResponse(const json::Dict& request,
                                     const RequestHandler& handler) const {
  const int request_id = request.at("id"s).AsInt();
  const std::string& type = request.at("type"s).AsString();

  if (type == "Bus"s) {
    return BuildBusResponse(request_id, request.at("name"s).AsString(),
                            handler);
  }
  if (type == "Stop"s) {
    return BuildStopResponse(request_id, request.at("name"s).AsString(),
                             handler);
  }
  if (type == "Map"s) {
    return BuildMapResponse(request_id, handler);
  }
  return BuildErrorResponse(request_id);
}

json::Document JsonReader::ProcessStatRequests(
    const json::Document& doc, const RequestHandler& handler) const {
  const json::Dict& root = doc.GetRoot().AsDict();

  json::Builder builder;
  auto array_context = builder.StartArray();

  auto it = root.find("stat_requests"s);
  if (it != root.end()) {
    for (const json::Node& node : it->second.AsArray()) {
      json::Node response = BuildResponse(node.AsDict(), handler);
      array_context.Value(std::move(response.GetValue()));
    }
  }

  return json::Document{array_context.EndArray().Build()};
}
