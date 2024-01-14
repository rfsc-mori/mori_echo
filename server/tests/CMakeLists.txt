add_executable(test_mori_echo_server)

# Tests Source
target_sources(
  test_mori_echo_server
  PRIVATE
    src/main.cpp
    src/business_rules.cpp
    src/client_authenticator/test_client_authenticator.cpp
    src/client_crypto/test_client_crypto.cpp
    src/message_receiver/test_message_receiver.cpp
    src/message_sender/test_message_sender.cpp
)

target_link_libraries(test_mori_echo_server PRIVATE mori_echo_server_lib ${Boost_LIBRARIES} spdlog::spdlog)

add_test(NAME business_rules COMMAND test_mori_echo_server -t business_rules)
