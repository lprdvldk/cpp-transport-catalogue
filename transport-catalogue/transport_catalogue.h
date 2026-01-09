#pragma once

#include "geo.h"
#include "detail.h"
#include <string>
#include <vector>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <format>
#include <functional>
#include <algorithm>

namespace database::transport_catalogue
{

    using namespace database::geo;
    using namespace database::detail;

    struct Stop
    {
        std::string name;
        Coordinates coords;
    };

    struct Bus
    {
        std::string name;
        std::vector<const Stop *> route;
    };

    struct BusInfoResult
    {
        size_t stops_on_route;
        size_t unique_stops;
        double route_length;
    };

    using BusesByStop = std::unordered_set<const Bus *>;

    class TransportCatalogue
    {
    public:
        explicit TransportCatalogue() = default;

        ~TransportCatalogue() = default;

        void AddBus(const std::string &name, const std::vector<std::string_view> &route_names);

        void AddStop(const std::string &name, Coordinates coords);

        const Bus *FindBus(std::string_view name) const;

        const Stop *FindStop(std::string_view name) const;

        std::pair<BusInfoResult, bool> BusInfo(std::string_view name) const;

        std::pair<BusesByStop, bool> StopInfo(std::string_view name) const;

    private:
        std::unordered_map<std::string_view, const Stop *, StringViewHash, StringViewEqual> stops_{};
        std::unordered_map<std::string_view, const Bus *, StringViewHash, StringViewEqual> buses_{};
        std::unordered_map<const Stop *, BusesByStop> buses_by_stops_{};

        std::deque<Bus> buses_deque_{};
        std::deque<Stop> stops_deque_{};

        uint64_t FindUniqueStops(const Bus *bus) const
        {
            std::unordered_set<std::string_view, StringViewHash, StringViewEqual> tmp;
            for (auto stop_ptr : bus->route)
            {
                tmp.insert(stop_ptr->name);
            }
            return tmp.size();
        }

        void AddBusToStopIndex(const Bus &bus)
        {
            for (const Stop *stop : bus.route)
            {
                if (buses_by_stops_.find(stop) != buses_by_stops_.end())
                {
                    auto &buses = buses_by_stops_.at(stop);
                    buses.insert(&bus);
                }
                else
                {
                    buses_by_stops_.insert({stop, {&bus}});
                }
            }
        }

        BusesByStop FindBusesByStop(const Stop *stop) const
        {
            auto ptr = buses_by_stops_.find(stop);
            if (ptr != buses_by_stops_.end())
            {
                return ptr->second;
            }
            return {};
        }
    };

} // transport_catalogue namespace