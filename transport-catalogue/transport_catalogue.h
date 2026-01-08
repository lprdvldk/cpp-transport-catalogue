#pragma once

#include "geo.h"
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

    struct StringViewHash
    {
        using is_transparent = void;

        size_t operator()(std::string_view sv) const noexcept
        {
            return std::hash<std::string_view>{}(sv);
        }
    };

    struct StringViewEqual
    {
        using is_transparent = void;

        bool operator()(std::string_view lhs, std::string_view rhs) const noexcept
        {
            return lhs == rhs;
        }
    };

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

    class TransportCatalogue
    {
        // Реализуйте класс самостоятельно
    public:
        explicit TransportCatalogue() = default;

        ~TransportCatalogue() = default;

        void AddBus(Bus bus);

        void AddStop(Stop stop);

        const Bus *FindBus(std::string_view name) const;

        const Stop *FindStop(std::string_view name) const;

        std::string BusInfo(std::string_view name) const;

        std::string StopInfo(std::string_view name) const;

        std::vector<const Bus *> FindBusesByStop(const Stop *stop) const;

    private:
        std::unordered_map<std::string_view, const Stop *, StringViewHash, StringViewEqual> stops_{};
        std::unordered_map<std::string_view, const Bus *, StringViewHash, StringViewEqual> buses_{};
        std::unordered_map<const Stop *, std::unordered_set<const Bus *>> buses_by_stops_{};

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
    };

} // transport_catalogue namespace