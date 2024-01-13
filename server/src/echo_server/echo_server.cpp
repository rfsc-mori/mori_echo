#include "echo_server/echo_server.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdint>
#include <spdlog/spdlog.h>

#include "client_channel/client_channel.hpp"
#include "message_receiver/message_receiver.hpp"

namespace mori_echo {

auto logger() -> std::shared_ptr<spdlog::logger> {
  static auto logger = spdlog::default_logger()->clone("echo_server");
  return logger;
}

auto handle_client(boost::asio::ip::tcp::socket socket)
    -> boost::asio::awaitable<void> {
  const auto client_id = socket.remote_endpoint().address().to_string();

  logger()->info("New client connected: {}", client_id);

  auto channel = client_channel{std::move(socket)};

  try {
    for (;;) {
      co_await receive_message(channel);
    }
  } catch (const boost::system::system_error& error) {
    if (error.code() != boost::asio::error::eof) {
      logger()->warn("Dropping client {}. Reason: {}", client_id, error.what());
    } else {
      logger()->info("Client {} disconnected.", client_id);
    }
  } catch (const std::exception& error) {
    logger()->warn("Dropping client {}. Reason: {}", client_id, error.what());
  }
}

auto tcp_listen(boost::asio::io_context& context, std::uint16_t port)
    -> boost::asio::awaitable<void> {
  auto acceptor = boost::asio::ip::tcp::acceptor{
      context, {boost::asio::ip::tcp::v4(), port}};

  logger()->info("Listening on port: {}", acceptor.local_endpoint().port());

  for (;;) {
    auto socket = co_await acceptor.async_accept(boost::asio::use_awaitable);

    boost::asio::co_spawn(context, handle_client(std::move(socket)),
                          boost::asio::detached);
  }
}

auto spawn_server(boost::asio::io_context& context, std::uint16_t port)
    -> void {
  boost::asio::co_spawn(context, tcp_listen(context, port),
                        [](std::exception_ptr error) {
                          if (error) {
                            std::rethrow_exception(error);
                          }
                        });
}

} // namespace mori_echo
