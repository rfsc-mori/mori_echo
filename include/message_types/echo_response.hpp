#pragma once

#include <cstdint>
#include <vector>

#include "message_base.hpp"

namespace mori_echo::messages {

struct echo_response : public message_base {
  std::uint16_t message_size;
  std::vector<std::byte> plain_message;
};

} // namespace mori_echo::messages
