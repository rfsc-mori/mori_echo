#pragma once

#include <cstdint>

#include "message_base.hpp"

namespace mori_echo::messages {

struct login_response : public message_base {
  std::uint16_t status_code = {};
};

} // namespace mori_echo::messages
