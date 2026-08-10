#include "json.h"
#include "json_reader.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include <iostream>
#include <iterator>
#include <string>

int main() {
  // Ускоряем ввод/вывод
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  // Читаем весь stdin в строку
  std::string input((std::istreambuf_iterator<char>(std::cin)),
                    std::istreambuf_iterator<char>());

  // Парсим JSON-документ
  json::Document doc = json::Parse(input);

  // Строим каталог
  database::transport_catalogue::TransportCatalogue catalogue;

  // Обработчик запросов (фасад)
  RequestHandler handler(catalogue);

  // Читаем JSON и обрабатываем запросы
  JsonReader reader(catalogue, handler);
  json::Node answer = reader.ProcessRequests(doc);

  // Выводим JSON-ответ
  json::Print(answer, std::cout);

  return 0;
}
