cmake_minimum_required(VERSION 3.25)
project(mori_echo_server)

# C++20
set(CMAKE_CXX_STANDARD 20)

# clangd intellisense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
## Common
include_directories(${CMAKE_SOURCE_DIR}/include)

## Boost
find_package(Boost 1.81.0 REQUIRED COMPONENTS context system)
include_directories(${Boost_INCLUDE_DIRS})

## Spdlog
find_package(spdlog CONFIG REQUIRED)
include_directories(${spdlog_INCLUDE_DIRS})

# Targets
add_executable(mori_echo_server ${SOURCES})
target_link_libraries(mori_echo_server PRIVATE ${Boost_LIBRARIES} spdlog::spdlog)

# Source
target_sources(
  mori_echo_server
  PRIVATE
    src/main.cpp
    src/echo_server/echo_server.hpp
    src/echo_server/echo_server.cpp
  PUBLIC
    ${CMAKE_SOURCE_DIR}/include/mori_echo/server_config.hpp
)
