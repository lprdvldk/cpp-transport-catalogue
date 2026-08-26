#include "map_renderer.h"

namespace renderer {

using namespace database::transport_catalogue;

std::unordered_map<const Bus*, svg::Color> MapRenderer::AssignBusColors(
    const std::vector<const Bus*>& sorted_buses) const {
  std::unordered_map<const Bus*, svg::Color> result;
  if (settings_.color_palette.empty()) {
    return result;
  }

  size_t color_index = 0;
  for (const Bus* bus : sorted_buses) {
    if (bus->route.empty()) {
      continue;
    }
    result[bus] =
        settings_.color_palette[color_index % settings_.color_palette.size()];
    ++color_index;
  }
  return result;
}

std::vector<const Stop*> MapRenderer::GetBusEndpoints(const Bus* bus) const {
  std::vector<const Stop*> endpoints;
  if (bus->route.empty()) {
    return endpoints;
  }

  const Stop* first = bus->route.front();
  endpoints.push_back(first);

  if (!bus->is_roundtrip) {
    const Stop* last = bus->route[bus->route.size() / 2];
    if (last != first) {
      endpoints.push_back(last);
    }
  }

  return endpoints;
}

std::map<std::string_view, const Stop*> MapRenderer::CollectUniqueStops(
    const std::vector<const Bus*>& sorted_buses) const {
  std::map<std::string_view, const Stop*> unique_stops;
  for (const Bus* bus : sorted_buses) {
    if (bus->route.empty()) {
      continue;
    }
    for (const Stop* stop : bus->route) {
      unique_stops[stop->name] = stop;
    }
  }
  return unique_stops;
}

void MapRenderer::RenderRouteLines(
    svg::Document& doc, const SphereProjector& projector,
    const std::vector<const Bus*>& sorted_buses,
    const std::unordered_map<const Bus*, svg::Color>& bus_colors) const {
  for (const Bus* bus : sorted_buses) {
    if (bus->route.empty()) {
      continue;
    }

    const svg::Color& color = bus_colors.at(bus);

    svg::Polyline polyline;
    polyline.SetFillColor(svg::NoneColor)
        .SetStrokeColor(color)
        .SetStrokeWidth(settings_.line_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    for (const Stop* stop : bus->route) {
      polyline.AddPoint(projector(stop->coords));
    }

    doc.Add(std::move(polyline));
  }
}

void MapRenderer::AddLabelPair(svg::Document& doc, svg::Point position,
                               const std::string& text, svg::Point offset,
                               int font_size, const svg::Color& label_fill,
                               bool bold) const {
  svg::Text substrate;
  substrate.SetPosition(position)
      .SetOffset(offset)
      .SetFontSize(static_cast<uint32_t>(font_size))
      .SetFontFamily("Verdana")
      .SetData(text)
      .SetFillColor(settings_.underlayer_color)
      .SetStrokeColor(settings_.underlayer_color)
      .SetStrokeWidth(settings_.underlayer_width)
      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
  if (bold) {
    substrate.SetFontWeight("bold");
  }
  doc.Add(std::move(substrate));

  svg::Text label;
  label.SetPosition(position)
      .SetOffset(offset)
      .SetFontSize(static_cast<uint32_t>(font_size))
      .SetFontFamily("Verdana")
      .SetData(text)
      .SetFillColor(label_fill);
  if (bold) {
    label.SetFontWeight("bold");
  }
  doc.Add(std::move(label));
}

void MapRenderer::RenderBusLabels(
    svg::Document& doc, const SphereProjector& projector,
    const std::vector<const Bus*>& sorted_buses,
    const std::unordered_map<const Bus*, svg::Color>& bus_colors) const {
  for (const Bus* bus : sorted_buses) {
    if (bus->route.empty()) {
      continue;
    }

    const svg::Color& color = bus_colors.at(bus);
    for (const Stop* stop : GetBusEndpoints(bus)) {
      AddLabelPair(doc, projector(stop->coords), bus->name,
                   settings_.bus_label_offset, settings_.bus_label_font_size,
                   color, true);
    }
  }
}

void MapRenderer::RenderStopCircles(
    svg::Document& doc, const SphereProjector& projector,
    const std::map<std::string_view, const Stop*>& unique_stops) const {
  for (const auto& entry : unique_stops) {
    const Stop* stop = entry.second;
    svg::Circle circle;
    circle.SetCenter(projector(stop->coords))
        .SetRadius(settings_.stop_radius)
        .SetFillColor(std::string("white"));
    doc.Add(std::move(circle));
  }
}

void MapRenderer::RenderStopLabels(
    svg::Document& doc, const SphereProjector& projector,
    const std::map<std::string_view, const Stop*>& unique_stops) const {
  for (const auto& entry : unique_stops) {
    const Stop* stop = entry.second;
    AddLabelPair(doc, projector(stop->coords), stop->name,
                 settings_.stop_label_offset, settings_.stop_label_font_size,
                 std::string("black"), false);
  }
}

svg::Document MapRenderer::RenderRouteMap(
    const std::vector<const Bus*>& sorted_buses) const {
  std::vector<database::geo::Coordinates> coords;
  for (const Bus* bus : sorted_buses) {
    for (const Stop* stop : bus->route) {
      coords.push_back(stop->coords);
    }
  }

  SphereProjector projector(coords.begin(), coords.end(), settings_.width,
                            settings_.height, settings_.padding);

  svg::Document doc;

  if (settings_.color_palette.empty()) {
    return doc;
  }

  const auto bus_colors = AssignBusColors(sorted_buses);
  const auto unique_stops = CollectUniqueStops(sorted_buses);

  RenderRouteLines(doc, projector, sorted_buses, bus_colors);
  RenderBusLabels(doc, projector, sorted_buses, bus_colors);
  RenderStopCircles(doc, projector, unique_stops);
  RenderStopLabels(doc, projector, unique_stops);

  return doc;
}

}  // namespace renderer
