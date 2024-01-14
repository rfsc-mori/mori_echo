cmake_minimum_required(VERSION 3.25)
project(mori_echo)

# Warnings
if (MSVC)
    # warning level 4
    add_compile_options(/W4)
else()
    # additional warnings
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_subdirectory(server)
