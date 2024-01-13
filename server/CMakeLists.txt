cmake_minimum_required(VERSION 3.25)
project(mori_echo_server)

# C++20
set(CMAKE_CXX_STANDARD 20)

# clangd intellisense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
## Common
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

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
    src/client_channel/client_channel.cpp
    src/echo_server/echo_server.cpp
    src/message_receiver/message_receiver.cpp
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/client_channel/client_channel.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/echo_server/echo_server.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/exceptions/client_error.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/message_receiver/message_receiver.hpp
  PUBLIC
    ${CMAKE_SOURCE_DIR}/include/mori_echo/endian_mode.hpp
    ${CMAKE_SOURCE_DIR}/include/mori_echo/server_config.hpp
    ${CMAKE_SOURCE_DIR}/include/message_types/echo_request.hpp
    ${CMAKE_SOURCE_DIR}/include/message_types/echo_response.hpp
    ${CMAKE_SOURCE_DIR}/include/message_types/login_request.hpp
    ${CMAKE_SOURCE_DIR}/include/message_types/login_response.hpp
    ${CMAKE_SOURCE_DIR}/include/message_types/message_base.hpp
    ${CMAKE_SOURCE_DIR}/include/message_types/message_header.hpp
    ${CMAKE_SOURCE_DIR}/include/message_types/message_type.hpp
    ${CMAKE_SOURCE_DIR}/include/mori_status/login_status.hpp
)
