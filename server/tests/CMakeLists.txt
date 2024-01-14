add_executable(test_mori_echo_server)

# Tests Source
target_sources(
  test_mori_echo_server
  PRIVATE
    src/main.cpp
)

target_link_libraries(test_mori_echo_server PRIVATE ${Boost_LIBRARIES} spdlog::spdlog)
