#pragma once

#include <array>

#include "message_base.hpp"
#include "mori_echo/server_config.hpp"

namespace mori_echo::messages {

struct login_request : public message_base {
  std::array<std::byte, config::username_size> username;
  std::array<std::byte, config::password_size> password;
};

} // namespace mori_echo::messages
