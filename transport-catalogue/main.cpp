#include <iostream>

#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    json::Document doc = json::Load(std::cin);

    database::transport_catalogue::TransportCatalogue catalogue;
    JsonReader reader(catalogue);
    reader.LoadBaseRequests(doc);

    renderer::RenderSettings settings = JsonReader::ParseRenderSettings(doc);
    renderer::MapRenderer map_renderer(settings);
    RequestHandler handler(catalogue, map_renderer);

    json::Print(reader.ProcessStatRequests(doc, handler), std::cout);

    return 0;
}
