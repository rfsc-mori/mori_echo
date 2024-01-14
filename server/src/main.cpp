#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <exception>
#include <spdlog/spdlog.h>

#include "client_authenticator/allow_all_client_authenticator.hpp"
#include "echo_server/echo_server.hpp"
#include "mori_echo/server_config.hpp"

auto log_fatal_error(const std::exception& error, int level = 0) -> void {
  if (level == 0) {
    spdlog::error("Fatal error: {}", error.what());
  } else {
    spdlog::error("{: >{}}Caused by: {}", "", level, error.what());
  }

  try {
    std::rethrow_if_nested(error);
  } catch (const std::exception& nested) {
    log_fatal_error(nested, level + 1);
  } catch (...) {
  }
}

auto main() -> int {
  spdlog::set_level(spdlog::level::debug);

  spdlog::info("MoriEcho TCP Echo Server started.");

  try {
    auto io_context = boost::asio::io_context{1};

    auto signals = boost::asio::signal_set{io_context, SIGINT, SIGTERM};
    signals.async_wait([&](auto, auto) { io_context.stop(); });

    mori_echo::spawn_server({
        .io_context = io_context,
        .port = mori_echo::config::tcp_port,

        .authenticator =
            mori_echo::auth::allow_all_client_authenticator::create(),
    });

    io_context.run();
  } catch (const std::exception& error) {
    log_fatal_error(error);
    return -1;
  }

  spdlog::info("MoriEcho TCP Echo Server exiting...");
}
