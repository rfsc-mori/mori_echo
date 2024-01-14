#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/test/framework.hpp>
#include <boost/test/unit_test.hpp>
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/spdlog.h>

#include "client_authenticator/allow_all_client_authenticator.hpp"
#include "client_authenticator/test_client_authenticator.hpp"
#include "client_channel/client_channel.hpp"
#include "client_crypto/test_client_crypto.hpp"
#include "echo_server/echo_server.hpp"
#include "message_receiver/message_receiver.hpp"
#include "message_sender/test_message_sender.hpp"
#include "message_types/echo_request.hpp"
#include "message_types/echo_response.hpp"
#include "message_types/login_request.hpp"
#include "message_types/login_response.hpp"
#include "mori_status/login_status.hpp"

namespace mori_echo::test {

inline constexpr auto test_tcp_port = std::uint16_t{31217};

BOOST_AUTO_TEST_SUITE(business_rules)

BOOST_AUTO_TEST_CASE(login_and_echo_decryption_enabled_success) {
  spdlog::set_level(spdlog::level::debug);

  auto logger = spdlog::default_logger()->clone(fmt::format(
      "test:{}",
      boost::unit_test::framework::current_test_case().p_name->c_str()));

  auto io_context = boost::asio::io_context{1};

  mori_echo::spawn_server(
      io_context.get_executor(),
      {
          .port = test_tcp_port,
          .enable_decryption = true,
          .authenticator =
              mori_echo::auth::allow_all_client_authenticator::create(),
      });

  boost::asio::co_spawn(
      io_context.get_executor(),
      [&]() -> boost::asio::awaitable<void> {
        const auto username = std::string{"testuser"};
        const auto password = std::string{"testpass"};

        auto socket = boost::asio::ip::tcp::socket{io_context};

        co_await socket.async_connect(
            {boost::asio::ip::address::from_string("127.0.0.1"), test_tcp_port},
            boost::asio::use_awaitable);

        auto channel = client_channel{std::move(socket)};

        constexpr auto login_request_sequence = 0;
        co_await send_message<messages::login_request>{}(
            channel, login_request_sequence, username, password);

        auto login_response_header = co_await receive_header(channel);
        BOOST_CHECK(login_response_header.type ==
                    messages::message_type::LOGIN_RESPONSE);
        BOOST_CHECK(login_response_header.sequence == login_request_sequence);

        const auto login_response =
            co_await receive_message<messages::login_response>(
                channel, std::move(login_response_header));

        logger->debug(
            "Login status code: {}",
            static_cast<
                std::underlying_type_t<decltype(login_response.status_code)>>(
                login_response.status_code));

        BOOST_CHECK(login_response.status_code ==
                    mori_status::login_status::OK);
        BOOST_CHECK(login_response.header.sequence == login_request_sequence);

        const auto echo_message = std::string{"This is a MoriEcho unit test."};

        auto echo_message_data = std::vector<std::byte>{echo_message.size()};

        std::transform(echo_message.begin(), echo_message.end(),
                       echo_message_data.begin(),
                       [](char each) { return static_cast<std::byte>(each); });

        constexpr auto echo_request_sequence = 1;

        const auto echo_message_encrypted = crypto::encrypt(
            {
                .username_sum = crypto::calculate_checksum(username),
                .password_sum = crypto::calculate_checksum(password),
                .sequence = echo_request_sequence,
            },
            echo_message_data);

        logger->debug("Requesting echo for encrypted message: {:X}",
                      spdlog::to_hex(echo_message_encrypted));

        co_await send_message<messages::echo_request>{}(
            channel, echo_request_sequence, echo_message_encrypted);

        auto echo_response_header = co_await receive_header(channel);
        BOOST_CHECK(echo_response_header.type ==
                    messages::message_type::ECHO_RESPONSE);
        BOOST_CHECK(echo_response_header.sequence == echo_request_sequence);

        auto echo_response = co_await receive_message<messages::echo_response>(
            channel, std::move(echo_response_header));
        BOOST_CHECK(echo_response.header.sequence == echo_request_sequence);
        BOOST_CHECK(echo_response.plain_message == echo_message_data);

        echo_response.plain_message.push_back(std::byte{'\0'});

        auto echo_response_string = std::string{
            reinterpret_cast<const char*>(echo_response.plain_message.data())};
        BOOST_CHECK(echo_response_string == echo_message);

        io_context.stop();
      },
      [](std::exception_ptr error) {
        if (error) {
          std::rethrow_exception(error);
        }
      });

  io_context.run();
}

BOOST_AUTO_TEST_CASE(login_and_echo_decryption_disabled_success) {
  spdlog::set_level(spdlog::level::debug);

  auto logger = spdlog::default_logger()->clone(fmt::format(
      "test:{}",
      boost::unit_test::framework::current_test_case().p_name->c_str()));

  auto io_context = boost::asio::io_context{1};

  mori_echo::spawn_server(
      io_context.get_executor(),
      {
          .port = test_tcp_port,
          .enable_decryption = false,
          .authenticator =
              mori_echo::auth::allow_all_client_authenticator::create(),
      });

  boost::asio::co_spawn(
      io_context.get_executor(),
      [&]() -> boost::asio::awaitable<void> {
        const auto username = std::string{"testuser"};
        const auto password = std::string{"testpass"};

        auto socket = boost::asio::ip::tcp::socket{io_context};

        co_await socket.async_connect(
            {boost::asio::ip::address::from_string("127.0.0.1"), test_tcp_port},
            boost::asio::use_awaitable);

        auto channel = client_channel{std::move(socket)};

        constexpr auto login_request_sequence = 0;
        co_await send_message<messages::login_request>{}(
            channel, login_request_sequence, username, password);

        auto login_response_header = co_await receive_header(channel);
        BOOST_CHECK(login_response_header.type ==
                    messages::message_type::LOGIN_RESPONSE);
        BOOST_CHECK(login_response_header.sequence == login_request_sequence);

        const auto login_response =
            co_await receive_message<messages::login_response>(
                channel, std::move(login_response_header));

        logger->debug(
            "Login status code: {}",
            static_cast<
                std::underlying_type_t<decltype(login_response.status_code)>>(
                login_response.status_code));

        BOOST_CHECK(login_response.status_code ==
                    mori_status::login_status::OK);
        BOOST_CHECK(login_response.header.sequence == login_request_sequence);

        const auto echo_message = std::string{"This is a MoriEcho unit test."};

        auto echo_message_data = std::vector<std::byte>{echo_message.size()};

        std::transform(echo_message.begin(), echo_message.end(),
                       echo_message_data.begin(),
                       [](char each) { return static_cast<std::byte>(each); });

        constexpr auto echo_request_sequence = 1;

        const auto echo_message_encrypted = crypto::encrypt(
            {
                .username_sum = crypto::calculate_checksum(username),
                .password_sum = crypto::calculate_checksum(password),
                .sequence = echo_request_sequence,
            },
            echo_message_data);

        logger->debug("Requesting echo for encrypted message: {:X}",
                      spdlog::to_hex(echo_message_encrypted));

        co_await send_message<messages::echo_request>{}(
            channel, echo_request_sequence, echo_message_encrypted);

        auto echo_response_header = co_await receive_header(channel);
        BOOST_CHECK(echo_response_header.type ==
                    messages::message_type::ECHO_RESPONSE);
        BOOST_CHECK(echo_response_header.sequence == echo_request_sequence);

        const auto echo_response =
            co_await receive_message<messages::echo_response>(
                channel, std::move(echo_response_header));
        BOOST_CHECK(echo_response.header.sequence == echo_request_sequence);
        BOOST_CHECK(echo_response.plain_message == echo_message_encrypted);

        io_context.stop();
      },
      [](std::exception_ptr error) {
        if (error) {
          std::rethrow_exception(error);
        }
      });

  io_context.run();
}

BOOST_AUTO_TEST_CASE(login_with_password) {
  spdlog::set_level(spdlog::level::debug);

  auto logger = spdlog::default_logger()->clone(fmt::format(
      "test:{}",
      boost::unit_test::framework::current_test_case().p_name->c_str()));

  auto io_context = boost::asio::io_context{1};

  mori_echo::spawn_server(
      io_context.get_executor(),
      {
          .port = test_tcp_port,
          .authenticator = mori_echo::auth::test_client_authenticator::create(),
      });

  boost::asio::co_spawn(
      io_context.get_executor(),
      [&]() -> boost::asio::awaitable<void> {
        const auto username = std::string{"testuser"};
        const auto password = std::string{"testpass"};

        auto socket = boost::asio::ip::tcp::socket{io_context};

        co_await socket.async_connect(
            {boost::asio::ip::address::from_string("127.0.0.1"), test_tcp_port},
            boost::asio::use_awaitable);

        auto channel = client_channel{std::move(socket)};

        constexpr auto login_request_sequence = 0;
        co_await send_message<messages::login_request>{}(
            channel, login_request_sequence, username, password);

        auto login_response_header = co_await receive_header(channel);
        BOOST_CHECK(login_response_header.type ==
                    messages::message_type::LOGIN_RESPONSE);
        BOOST_CHECK(login_response_header.sequence == login_request_sequence);

        const auto login_response =
            co_await receive_message<messages::login_response>(
                channel, std::move(login_response_header));

        logger->debug(
            "Login status code: {}",
            static_cast<
                std::underlying_type_t<decltype(login_response.status_code)>>(
                login_response.status_code));

        BOOST_CHECK(login_response.status_code ==
                    mori_status::login_status::OK);
        BOOST_CHECK(login_response.header.sequence == login_request_sequence);

        io_context.stop();
      },
      [](std::exception_ptr error) {
        if (error) {
          std::rethrow_exception(error);
        }
      });

  io_context.run();
}

BOOST_AUTO_TEST_CASE(login_failure) {
  spdlog::set_level(spdlog::level::debug);

  auto logger = spdlog::default_logger()->clone(fmt::format(
      "test:{}",
      boost::unit_test::framework::current_test_case().p_name->c_str()));

  auto io_context = boost::asio::io_context{1};

  mori_echo::spawn_server(
      io_context.get_executor(),
      {
          .port = test_tcp_port,
          .authenticator = mori_echo::auth::test_client_authenticator::create(),
      });

  boost::asio::co_spawn(
      io_context.get_executor(),
      [&]() -> boost::asio::awaitable<void> {
        const auto username = std::string{"testuser"};
        const auto password = std::string{"wrong_password"};

        auto socket = boost::asio::ip::tcp::socket{io_context};

        co_await socket.async_connect(
            {boost::asio::ip::address::from_string("127.0.0.1"), test_tcp_port},
            boost::asio::use_awaitable);

        auto channel = client_channel{std::move(socket)};

        constexpr auto login_request_sequence = 0;
        co_await send_message<messages::login_request>{}(
            channel, login_request_sequence, username, password);

        auto login_response_header = co_await receive_header(channel);
        BOOST_CHECK(login_response_header.type ==
                    messages::message_type::LOGIN_RESPONSE);
        BOOST_CHECK(login_response_header.sequence == login_request_sequence);

        const auto login_response =
            co_await receive_message<messages::login_response>(
                channel, std::move(login_response_header));

        logger->debug(
            "Login status code: {}",
            static_cast<
                std::underlying_type_t<decltype(login_response.status_code)>>(
                login_response.status_code));

        BOOST_CHECK(login_response.status_code ==
                    mori_status::login_status::FAILED);
        BOOST_CHECK(login_response.header.sequence == login_request_sequence);

        const auto echo_message = std::string{"This is a MoriEcho unit test."};

        auto echo_message_data = std::vector<std::byte>{echo_message.size()};

        std::transform(echo_message.begin(), echo_message.end(),
                       echo_message_data.begin(),
                       [](char each) { return static_cast<std::byte>(each); });

        constexpr auto echo_request_sequence = 1;

        const auto echo_message_encrypted = crypto::encrypt(
            {
                .username_sum = crypto::calculate_checksum(username),
                .password_sum = crypto::calculate_checksum(password),
                .sequence = echo_request_sequence,
            },
            echo_message_data);

        logger->debug("Requesting echo for encrypted message: {:X}",
                      spdlog::to_hex(echo_message_encrypted));

        BOOST_CHECK_EXCEPTION(
            co_await send_message<messages::echo_request>{}(
                channel, echo_request_sequence, echo_message_encrypted),
            boost::system::system_error,
            [](const boost::system::system_error& error) {
              return error.code() == boost::asio::error::connection_reset ||
                     error.code() == boost::asio::error::broken_pipe;
            });

        io_context.stop();
      },
      [](std::exception_ptr error) {
        if (error) {
          std::rethrow_exception(error);
        }
      });

  io_context.run();
}

BOOST_AUTO_TEST_CASE(echo_failure_without_login) {
  spdlog::set_level(spdlog::level::debug);

  auto logger = spdlog::default_logger()->clone(fmt::format(
      "test:{}",
      boost::unit_test::framework::current_test_case().p_name->c_str()));

  auto io_context = boost::asio::io_context{1};

  mori_echo::spawn_server(
      io_context.get_executor(),
      {
          .port = test_tcp_port,
          .authenticator =
              mori_echo::auth::allow_all_client_authenticator::create(),
      });

  boost::asio::co_spawn(
      io_context.get_executor(),
      [&]() -> boost::asio::awaitable<void> {
        const auto username = std::string{"testuser"};
        const auto password = std::string{"testpass"};

        auto socket = boost::asio::ip::tcp::socket{io_context};

        co_await socket.async_connect(
            {boost::asio::ip::address::from_string("127.0.0.1"), test_tcp_port},
            boost::asio::use_awaitable);

        auto channel = client_channel{std::move(socket)};

        const auto echo_message = std::string{"This is a MoriEcho unit test."};

        auto echo_message_data = std::vector<std::byte>{echo_message.size()};

        std::transform(echo_message.begin(), echo_message.end(),
                       echo_message_data.begin(),
                       [](char each) { return static_cast<std::byte>(each); });

        constexpr auto echo_request_sequence = 1;

        const auto echo_message_encrypted = crypto::encrypt(
            {
                .username_sum = crypto::calculate_checksum(username),
                .password_sum = crypto::calculate_checksum(password),
                .sequence = echo_request_sequence,
            },
            echo_message_data);

        logger->debug("Requesting echo for encrypted message: {:X}",
                      spdlog::to_hex(echo_message_encrypted));

        BOOST_CHECK_EXCEPTION(
            co_await send_message<messages::echo_request>{}(
                channel, echo_request_sequence, echo_message_encrypted),
            boost::system::system_error,
            [](const boost::system::system_error& error) {
              return error.code() == boost::asio::error::connection_reset ||
                     error.code() == boost::asio::error::broken_pipe;
            });

        io_context.stop();
      },
      [](std::exception_ptr error) {
        if (error) {
          std::rethrow_exception(error);
        }
      });

  io_context.run();
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace mori_echo::test
