#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <exception>
#include <spdlog/spdlog.h>

#include "echo_server/echo_server.hpp"
#include "mori_echo/server_config.hpp"

auto main() -> int {
  spdlog::info("MoriEcho TCP Echo Server started.");

  try {
    auto io_context = boost::asio::io_context{};

    auto signals = boost::asio::signal_set{io_context, SIGINT, SIGTERM};
    signals.async_wait([&](auto, auto) { io_context.stop(); });

    mori_echo::spawn_server({
        .io_context = io_context,
        .port = mori_echo::config::tcp_port,
    });

    io_context.run();
  } catch (const std::exception& error) {
    spdlog::error("Fatal error: {}", error.what());
    return -1;
  }

  spdlog::info("MoriEcho TCP Echo Server exiting...");
}
