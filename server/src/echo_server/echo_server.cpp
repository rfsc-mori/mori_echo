#include "echo_server/echo_server.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <exception>
#include <functional>
#include <spdlog/spdlog.h>

#include "client_channel/client_channel.hpp"
#include "client_session/client_session.hpp"
#include "message_receiver/message_receiver.hpp"

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

[[nodiscard]] auto make_client_session(boost::asio::ip::tcp::endpoint endpoint)
    -> client_session {
  return {
      .uuid = boost::uuids::to_string(boost::uuids::random_generator{}()),
      .address = endpoint.address().to_string(),

      .is_logged_in = false,
  };
}

[[nodiscard]] auto handle_client(boost::asio::ip::tcp::socket socket)
    -> boost::asio::awaitable<void> {
  auto session = make_client_session(socket.remote_endpoint());

  logger()->info("New client connected: {}", session.uuid);

  auto channel = client_channel{std::move(socket)};

  try {
    for (;;) {
      co_await receive_message(channel);
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

    boost::asio::co_spawn(ctx.io_context, handle_client(std::move(socket)),
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
