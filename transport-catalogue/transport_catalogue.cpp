#include "transport_catalogue.h"

namespace database::transport_catalogue
{

    void TransportCatalogue::AddBus(const std::string &name, const std::vector<std::string_view> &route_names)
    {
        std::vector<const Stop *> tmp;
        for (const std::string_view &name : route_names)
        {
            tmp.push_back(FindStop(name));
        }
        Bus bus{name, std::move(tmp)};

        buses_deque_.push_back(std::move(bus));
        const Bus &stored_bus = buses_deque_.back();

        buses_.emplace(stored_bus.name, &stored_bus);

        AddBusToStopIndex(stored_bus);
    }

    void TransportCatalogue::AddStop(const std::string &name, Coordinates coords)
    {
        Stop stop{name, coords};

        stops_deque_.push_back(std::move(stop));
        const Stop &stored_stop = stops_deque_.back();

        stops_.emplace(stored_stop.name, &stored_stop);
    }

    BusInfoResult TransportCatalogue::BusInfo(std::string_view name) const
    {
        const Bus *bus = FindBus(name);

        if (!bus)
        {
            return {false, name};
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
        return {
            true,
            bus->name,
            num_stops,
            num_unique_stops,
            route_length};
    }

    StopInfoResult TransportCatalogue::StopInfo(std::string_view name) const
    {
        const Stop *stop = FindStop(name);

        if (!stop)
        {
            return {false, false, name, {}};
        }

        auto busses = FindBusesByStop(stop);

        if (busses.size() == 0)
        {
            return {
                true,
                false,
                name,
                {}};
        }
        return {
            true,
            true,
            name,
            busses};
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