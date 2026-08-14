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
  JsonReader reader(catalogue);
  reader.LoadBaseRequests(doc);

  renderer::RenderSettings settings = JsonReader::ParseRenderSettings(doc);
  renderer::MapRenderer map_renderer(settings);
  RequestHandler handler(catalogue, map_renderer);

  json::Print(reader.ProcessStatRequests(doc, handler), std::cout);

  return 0;
}
