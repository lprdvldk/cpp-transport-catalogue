#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <regex>
#include <iterator>

namespace database::input {

using namespace geo;

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта,
 * долгота)
 */
Coordinates ParseCoordinates(std::string_view str) {
  static const double nan = std::nan("");

  auto not_space = str.find_first_not_of(' ');
  auto comma = str.find(',');

  if (comma == str.npos) {
    return {nan, nan};
  }

  auto not_space2 = str.find_first_not_of(' ', comma + 1);

  double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
  double lng = std::stod(std::string(str.substr(not_space2)));

  return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
  const auto start = string.find_first_not_of(' ');
  if (start == string.npos) {
    return {};
  }
  return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя
 * delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
  std::vector<std::string_view> result;

  size_t pos = 0;
  while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
    auto delim_pos = string.find(delim, pos);
    if (delim_pos == string.npos) {
      delim_pos = string.size();
    }
    if (auto substr = Trim(string.substr(pos, delim_pos - pos));
        !substr.empty()) {
      result.push_back(substr);
    }
    pos = delim_pos + 1;
  }

  return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок
 * [A,B,C,A] Для некольцевого маршрута (A-B-C-D) возвращает массив названий
 * остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
  if (route.find('>') != route.npos) {
    return Split(route, '>');
  }

  auto stops = Split(route, '-');
  std::vector<std::string_view> results(stops.begin(), stops.end());
  results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

  return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
  auto colon_pos = line.find(':');
  if (colon_pos == line.npos) {
    return {};
  }

  auto space_pos = line.find(' ');
  if (space_pos >= colon_pos) {
    return {};
  }

  auto not_space = line.find_first_not_of(' ', space_pos);
  if (not_space >= colon_pos) {
    return {};
  }

  return {std::string(line.substr(0, space_pos)),
          std::string(line.substr(not_space, colon_pos - not_space)),
          std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
  auto command_description = ParseCommandDescription(line);
  if (command_description) {
    commands_.push_back(std::move(command_description));
  }
}

std::vector<std::pair<std::string, int>> ParseDistances(std::string_view distances_str) {
    std::vector<std::pair<std::string, int>> distances;
    
    std::regex pattern(R"(\s*(\d+)\s*m\s+to\s+([^,]+))");
    
    std::string str(distances_str);
    std::sregex_iterator begin(str.begin(), str.end(), pattern);
    std::sregex_iterator end;
    
    for (auto it = begin; it != end; ++it) {
        auto match = *it;
        if (match.size() >= 3) {
          int distance = std::stoi(match[1].str());
          std::string stop_name = match[2].str();
          
          stop_name.erase(stop_name.find_last_not_of(" \t\n\r") + 1);
          
          if (!stop_name.empty() && distance > 0) {
              distances.emplace_back(std::move(stop_name), distance);
          }
        }
    }
    
    return distances;
}

void InputReader::ApplyCommands(
    TransportCatalogue &catalogue) const {

  for (const CommandDescription &command : commands_) {
    if (command.command == "Stop") {
      auto first_comma = command.description.find(',');
      if (first_comma == std::string::npos) {
        continue;
      }
      auto second_comma = command.description.find(',', first_comma + 1);

      auto coords_str = command.description.substr(0, second_comma);
      
      catalogue.AddStop(command.id, ParseCoordinates(coords_str));
      
    }
  }

  for (const CommandDescription &command : commands_) {
    if (command.command == "Bus") {
      auto route_names = ParseRoute(command.description);
      catalogue.AddBus(command.id, route_names);
    }
  }
  
  for (const CommandDescription &command : commands_) {
    if (command.command == "Stop") {
      auto first_comma = command.description.find(',');
      if (first_comma == std::string::npos) continue;
      
      auto second_comma = command.description.find(',', first_comma + 1);
      if (second_comma == std::string::npos) continue;
      
      auto distances_str = command.description.substr(second_comma + 1);
      if (distances_str.empty()) continue;
      
      auto distances = ParseDistances(distances_str);
      auto from_stop = catalogue.FindStop(command.id);
      
      for (const auto& [to_stop_name, distance] : distances) {
        auto to_stop = catalogue.FindStop(to_stop_name);
        if (from_stop && to_stop) {
          catalogue.AddStopDistance(from_stop, to_stop, distance);
        }
      }
    }
  }
}
} // namespace database::input