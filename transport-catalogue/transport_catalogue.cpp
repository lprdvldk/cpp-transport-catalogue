#include "transport_catalogue.h"

namespace database::transport_catalogue
{

    void TransportCatalogue::AddBus(Bus bus)
    {
        buses_deque_.push_back(std::move(bus));
        const Bus &stored_bus = buses_deque_.back();

        buses_.emplace(stored_bus.name, &stored_bus);

        AddBusToStopIndex(stored_bus);
    }

    void TransportCatalogue::AddStop(Stop stop)
    {
        stops_deque_.push_back(std::move(stop));
        const Stop &stored_stop = stops_deque_.back();

        stops_.emplace(stored_stop.name, &stored_stop);
    }

    std::string TransportCatalogue::BusInfo(std::string_view name) const
    {
        const Bus *bus = FindBus(name);

        if (!bus)
        {
            return std::format("Bus {}: not found", name);
        }

        auto num_stops = bus->route.size();
        auto num_unique_stops = FindUniqueStops(bus);

        auto route_length = 0.0;
        Coordinates first;
        Coordinates second;
        auto is_first = true;

        for (const Stop *stop_ptr : bus->route)
        {
            if (is_first)
            {
                first = stop_ptr->coords;
                is_first = false;
            }

            second = stop_ptr->coords;
            route_length += ComputeDistance(first, second);
            first = second;
        }

        return std::format("Bus {}: {} stops on route, {} unique stops, {:.6g} route length",
                           bus->name, num_stops, num_unique_stops, route_length);
    }

    std::string TransportCatalogue::StopInfo(std::string_view name) const
    {
        const Stop *stop = FindStop(name);

        if (!stop)
        {
            return std::format("Stop {}: not found", name);
        }

        auto buses = FindBusesByStop(stop);
        if (buses.size() == 0)
        {
            return std::format("Stop {}: no buses", name);
        }

        // std::string result = "Stop " + stop->name + ": ";
        std::string result = std::format("Stop {}: buses", name);
        for (auto bus : buses)
        {
            result.append(" " + bus->name);
        }
        return result;
    }

    std::vector<const Bus *> TransportCatalogue::FindBusesByStop(const Stop *stop) const
    {

        auto comparator = [](auto lhs, auto rhs)
        {
            return lhs->name <= rhs->name;
        };

        auto ptr = buses_by_stops_.find(stop);
        if (ptr != buses_by_stops_.end())
        {
            std::vector<const Bus *> tmp;
            for (auto bus : ptr->second)
            {
                tmp.push_back(bus);
            }
            std::sort(tmp.begin(), tmp.end(), comparator);
            return tmp;
        }
        return {};
    }

    const Bus *TransportCatalogue::FindBus(std::string_view name) const
    {
        auto it = buses_.find(name);
        if (it != buses_.end())
        {
            return it->second;
        }
        return nullptr;
    }

    const Stop *TransportCatalogue::FindStop(std::string_view name) const
    {
        auto it = stops_.find(name);
        if (it != stops_.end())
        {
            return it->second;
        }
        return nullptr;
    }

} // transport_catalogue namespace