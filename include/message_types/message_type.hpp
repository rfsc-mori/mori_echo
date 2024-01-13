#pragma once

#include <cstdint>

namespace mori_echo::messages {

enum class message_type : uint8_t {
  LOGIN_REQUEST = 0,
  LOGIN_RESPONSE = 1,
  ECHO_REQUEST = 2,
  ECHO_RESPONSE = 3
};

} // namespace mori_echo::messages
