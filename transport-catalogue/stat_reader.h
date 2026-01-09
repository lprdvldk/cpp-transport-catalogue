#pragma once

#include <iosfwd>
#include "transport_catalogue.h"
#include <format>

namespace database::output
{

    using namespace transport_catalogue;

    void PrintBusInfo(const TransportCatalogue &transport_catalogue, std::string_view name, std::ostream &output);

    void PrintStopInfo(const TransportCatalogue &transport_catalogue, std::string_view name, std::ostream &output);

    void ParseAndPrintStat(const TransportCatalogue &transport_catalogue, std::string_view request,
                           std::ostream &output);

}