#pragma once

#include <string>

#include "message_base.hpp"

namespace mori_echo::messages {

struct login_request : public message_base {
  std::string username;
  std::string password;
};

} // namespace mori_echo::messages
