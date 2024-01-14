#include "echo_server/echo_server.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <exception>
#include <functional>
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/spdlog.h>

#include "client_channel/client_channel.hpp"
#include "client_crypto/client_crypto.hpp"
#include "client_session/client_session.hpp"
#include "exceptions/client_error.hpp"
#include "message_receiver/message_receiver.hpp"
#include "message_sender/message_sender.hpp"
#include "message_types/echo_request.hpp"
#include "message_types/echo_response.hpp"
#include "message_types/login_request.hpp"
#include "message_types/login_response.hpp"
#include "mori_echo/server_config.hpp"
#include "mori_status/login_status.hpp"

namespace mori_echo {

[[nodiscard]] auto logger() -> std::shared_ptr<spdlog::logger> {
  static auto logger = spdlog::default_logger()->clone("echo_server");
  return logger;
}

auto log_client_error(const std::exception& error,
                      const client_session& session, int level = 0) -> void {
  if (level == 0) {
    logger()->warn("Dropping client {}. Reason: {}", session.uuid,
                   error.what());
  } else {
    logger()->warn("{: >{}}Caused by: {}", "", level, error.what());
  }

  try {
    std::rethrow_if_nested(error);
  } catch (const std::exception& nested) {
    log_client_error(nested, session, level + 1);
  } catch (...) {
  }
}

auto log_decrypted_message(const client_session& session,
                           const std::vector<std::byte>& plain) -> void {
  auto text = std::vector<char>{};
  text.reserve(plain.size());

  std::transform(plain.begin(), plain.end(), std::back_inserter(text),
                 [](std::byte each) { return static_cast<char>(each); });

  logger()->debug("Echoing decrypted message from {}: {}", session.uuid,
                  fmt::join(text, ""));
}

auto log_encrypted_message(const client_session& session,
                           const std::vector<std::byte>& encrypted) -> void {
  logger()->debug("Echoing encrypted message from {}: {:X}", session.uuid,
                  spdlog::to_hex(encrypted));
}

[[nodiscard]] auto handle_authenticated_client(client_channel& channel,
                                               client_session& session)
    -> boost::asio::awaitable<void> {
  auto header = co_await receive_header(channel);

  switch (header.type) {
    case messages::message_type::ECHO_REQUEST: {
      const auto echo = co_await receive_message<messages::echo_request>(
          channel, std::move(header));

      if constexpr (config::enable_decryption) {
        const auto plain_message = crypto::decrypt(
            {
                .username_sum = session.username_sum,
                .password_sum = session.password_sum,
                .sequence = echo.header.sequence,
            },
            std::move(echo.cipher_message));

        log_decrypted_message(session, plain_message);

        co_await send_message<messages::echo_response>{}(
            channel, echo.header.sequence, plain_message);
      } else {
        log_encrypted_message(session, echo.cipher_message);

        co_await send_message<messages::echo_response>{}(
            channel, echo.header.sequence, echo.cipher_message);
      }
    } break;

    case messages::message_type::LOGIN_RESPONSE:
    case messages::message_type::ECHO_RESPONSE:
      throw exceptions::client_error{
          "The client should never send this message."};

    case messages::message_type::LOGIN_REQUEST:
      throw exceptions::client_error{"The client is already logged in."};
  }
}

[[nodiscard]] auto
handle_new_client(client_channel& channel, client_session& session,
                  std::shared_ptr<auth::client_authenticator> authenticator)
    -> boost::asio::awaitable<void> {
  auto header = co_await receive_header(channel);

  if (header.type != messages::message_type::LOGIN_REQUEST) {
    throw exceptions::client_error{"The client is not logged in."};
  }

  const auto login = co_await receive_message<messages::login_request>(
      channel, std::move(header));

  auto authentication_error = std::exception_ptr{};

  try {
    authenticator->authenticate(login.username, login.password);

    session.username_sum = crypto::calculate_checksum(login.username);
    session.password_sum = crypto::calculate_checksum(login.password);

    session.is_logged_in = true;
  } catch (const std::exception& error) {
    authentication_error = std::current_exception();
  }

  if (!authentication_error) {
    co_await send_message<messages::login_response>{}(
        channel, login.header.sequence, mori_status::login_status::OK);
  } else {
    co_await send_message<messages::login_response>{}(
        channel, login.header.sequence, mori_status::login_status::FAILED);

    try {
      std::rethrow_exception(authentication_error);
    } catch (const std::exception& error) {
      std::throw_with_nested(
          exceptions::client_error{"The client login failed."});
    }
  }
}

[[nodiscard]] auto make_client_session(boost::asio::ip::tcp::endpoint endpoint)
    -> client_session {
  return {
      .uuid = boost::uuids::to_string(boost::uuids::random_generator{}()),
      .address = endpoint.address().to_string(),

      .is_logged_in = false,
  };
}

[[nodiscard]] auto
handle_client(boost::asio::ip::tcp::socket socket,
              std::shared_ptr<auth::client_authenticator> authenticator)
    -> boost::asio::awaitable<void> {
  auto session = make_client_session(socket.remote_endpoint());

  logger()->info("New client connected: {}", session.uuid);

  auto channel = client_channel{std::move(socket)};

  try {
    for (;;) {
      if (session.is_logged_in) {
        co_await handle_authenticated_client(channel, session);
      } else {
        co_await handle_new_client(channel, session, std::move(authenticator));
      }
    }
  } catch (const boost::system::system_error& error) {
    if (error.code() != boost::asio::error::eof) {
      log_client_error(error, session);
    } else {
      logger()->info("Client {} disconnected.", session.uuid);
    }
  } catch (const std::exception& error) {
    log_client_error(error, session);
  }
}

[[nodiscard]] auto tcp_listen(echo_server_context ctx)
    -> boost::asio::awaitable<void> {
  auto acceptor = boost::asio::ip::tcp::acceptor{
      ctx.io_context, {boost::asio::ip::tcp::v4(), ctx.port}};

  logger()->info("Listening on port: {}", acceptor.local_endpoint().port());

  for (;;) {
    auto socket = co_await acceptor.async_accept(boost::asio::use_awaitable);

    boost::asio::co_spawn(ctx.io_context,
                          handle_client(std::move(socket), ctx.authenticator),
                          [](std::exception_ptr error) {
                            if (error) {
                              std::rethrow_exception(error);
                            }
                          });
  }
}

auto spawn_server(echo_server_context ctx) -> void {
  auto io_context = std::ref(ctx.io_context);

  boost::asio::co_spawn(io_context.get(), tcp_listen(std::move(ctx)),
                        [](std::exception_ptr error) {
                          if (error) {
                            std::rethrow_exception(error);
                          }
                        });
}

} // namespace mori_echo
