#include "stat_reader.h"

#include <ostream>

namespace database::output {

// Used, because i have to compare refs' names. Don't know how to use std::less
// in this case.
auto comparator = [](auto lhs, auto rhs) { return lhs->name < rhs->name; };

void PrintBusInfo(const TransportCatalogue &transport_catalogue,
                  std::string_view name, std::ostream &output) {
  auto info = transport_catalogue.BusInfo(name);
  if (!info.has_value()) {
    output << std::format("Bus {}: not found", name) << "\n";
    return;
  }
  auto bus = *info;

  output
      << std::format(
             "Bus {}: {} stops on route, {} unique stops, {:.6g} route length",
             name, bus.stops_on_route, bus.unique_stops, bus.route_length)
      << "\n";
}

void PrintStopInfo(const TransportCatalogue &transport_catalogue,
                   std::string_view name, std::ostream &output) {
  auto info = transport_catalogue.StopInfo(name);
  if (!info.has_value()) {
    output << std::format("Stop {}: not found", name) << "\n";
    return;
  }

  auto busses = *info;
  if (busses.size() == 0) {
    output << std::format("Stop {}: no buses", name) << "\n";
    return;
  }

  std::vector<const Bus *> tmp;
  for (auto bus : busses) {
    tmp.push_back(bus);
  }
  std::sort(tmp.begin(), tmp.end(), comparator);

  std::string result = std::format("Stop {}: buses", name);
  output << result;
  for (auto bus : tmp) {
    output << " " + bus->name;
  }
  output << "\n";
}

void ParseAndPrintStat(const TransportCatalogue &transport_catalogue,
                       std::string_view request, std::ostream &output) {
  auto space_pos = request.find(' ');
  auto command = request.substr(0, space_pos);
  std::string_view name = request.substr(space_pos + 1);

  if (command == "Bus") {
    PrintBusInfo(transport_catalogue, name, output);
  } else if (command == "Stop") {
    PrintStopInfo(transport_catalogue, name, output);
  }
}
} // namespace database::output