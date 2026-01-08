#pragma once

#include <iosfwd>
#include "transport_catalogue.h"


namespace database::output{

void ParseAndPrintStat(const database::transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream &output);

}