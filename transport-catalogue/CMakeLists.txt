cmake_minimum_required(VERSION 3.16)
project(TransportCatalogue CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(transport_catalogue
    main.cpp
    domain.h
    domain.cpp
    geo.h
    geo.cpp
    json.h
    json.cpp
    json_reader.h
    json_reader.cpp
    request_handler.h
    request_handler.cpp
    transport_catalogue.h
    transport_catalogue.cpp
    map_renderer.h
    map_renderer.cpp
    svg.h
    svg.cpp
)

if(MSVC)
    target_compile_options(transport_catalogue PRIVATE /W4)
else()
    target_compile_options(transport_catalogue PRIVATE -Wall -Wextra)
endif()
