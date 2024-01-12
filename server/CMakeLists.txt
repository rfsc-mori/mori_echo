cmake_minimum_required(VERSION 3.25)
project(mori_echo_server)

# C++20
set(CMAKE_CXX_STANDARD 20)

# clangd intellisense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Targets
add_executable(mori_echo_server ${SOURCES})

# Source
target_sources(
  mori_echo_server
  PRIVATE
    src/main.cpp
)
