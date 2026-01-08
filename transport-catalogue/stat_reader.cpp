#include "stat_reader.h"

#include <ostream>

namespace database::output{

void ParseAndPrintStat(const database::transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream &output)
{
    // Реализуйте самостоятельно
    auto space_pos = request.find(' ');
    auto command = request.substr(0, space_pos);
    if (command == "Bus")
    {
        std::string_view bus_id = request.substr(space_pos + 1);
        output << transport_catalogue.BusInfo(bus_id) << std::endl;
    }
    else if (command == "Stop")
    {
        std::string_view stop_id = request.substr(space_pos + 1);
        output << transport_catalogue.StopInfo(stop_id) << std::endl;
    }
}
}