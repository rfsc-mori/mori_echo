#pragma once

#include <cstdint>

#include "message_type.hpp"

namespace mori_echo::messages {

struct message_header {
  uint16_t total_size;
  message_type type;
  uint8_t sequence;
};

} // namespace mori_echo::messages
