#include "stat_reader.h"

#include <ostream>

namespace database::output
{

    using namespace transport_catalogue;

    auto comparator = [](auto lhs, auto rhs)
    {
        return lhs->name < rhs->name;
    };

    void ParseAndPrintStat(const TransportCatalogue &transport_catalogue, std::string_view request,
                           std::ostream &output)
    {
        auto space_pos = request.find(' ');
        auto command = request.substr(0, space_pos);
        if (command == "Bus")
        {
            std::string_view bus_id = request.substr(space_pos + 1);
            auto info = transport_catalogue.BusInfo(bus_id);
            if (!info.found)
            {
                output << std::format("Bus {}: not found", info.name) << "\n";
                return;
            }

            output << std::format(
                          "Bus {}: {} stops on route, {} unique stops, {:.6g} route length",
                          info.name, info.stops_on_route, info.unique_stops, info.route_length)
                   << "\n";
        }
        else if (command == "Stop")
        {
            std::string_view stop_id = request.substr(space_pos + 1);
            auto info = transport_catalogue.StopInfo(stop_id);
            if (!info.found)
            {
                output << std::format("Stop {}: not found", info.name) << "\n";
                return;
            }

            if (!info.has_busses)
            {
                output << std::format("Stop {}: no buses", info.name) << "\n";
                return;
            }

            std::vector<const Bus *> tmp;
            for (auto bus : info.busses)
            {
                tmp.push_back(bus);
            }
            std::sort(tmp.begin(), tmp.end(), comparator);

            std::string result = std::format("Stop {}: buses", info.name);
            output << result;
            for (auto bus : tmp)
            {
                output << " " + bus->name;
            }
            output << "\n";
        }
    }
}