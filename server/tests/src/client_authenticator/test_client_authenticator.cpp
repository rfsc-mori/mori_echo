#include "test_client_authenticator.hpp"

#include <spdlog/spdlog.h>

#include "exceptions/client_error.hpp"

namespace mori_echo::auth {

[[nodiscard]] inline auto logger() -> std::shared_ptr<spdlog::logger> {
  static auto logger =
      spdlog::default_logger()->clone("test_client_authenticator");
  return logger;
}

auto test_client_authenticator::authenticate(std::string_view username,
                                             std::string_view password)
    -> void {
  if (username == "testuser" && password == "testpass") {
    logger()->info("Allowing user: {}", username);
  } else {
    logger()->info("Denying user: {}", username);
    throw exceptions::client_error("Invalid username or password.");
  }
}

} // namespace mori_echo::auth
