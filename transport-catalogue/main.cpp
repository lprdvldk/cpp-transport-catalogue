#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include <iostream>
#include <iterator>
#include <string>

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  std::string input((std::istreambuf_iterator<char>(std::cin)),
                    std::istreambuf_iterator<char>());

  json::Document doc = json::Parse(input);

  database::transport_catalogue::TransportCatalogue catalogue;
  RequestHandler handler(catalogue);
  JsonReader reader(catalogue, handler);

  reader.LoadBaseRequests(doc);
  renderer::RenderSettings settings = reader.ParseRenderSettingsFromDocument(doc);

  renderer::MapRenderer map_renderer(settings);
  svg::Document map = map_renderer.RenderRouteMap(catalogue.GetAllBuses());
  map.Render(std::cout);

  return 0;
}
