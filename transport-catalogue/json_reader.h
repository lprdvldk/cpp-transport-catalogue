#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include <string>

class JsonReader {
public:
    JsonReader(database::transport_catalogue::TransportCatalogue& catalogue,
               const RequestHandler& handler);

    json::Node ProcessRequests(const json::Node& document);

private:
    database::transport_catalogue::TransportCatalogue& catalogue_;
    const RequestHandler& handler_;

    void ProcessBaseRequests(const json::Node::Array& base_requests);
    json::Node::Array ProcessStatRequests(const json::Node::Array& stat_requests);
};
