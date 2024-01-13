#pragma once

#include "message_header.hpp"

namespace mori_echo::messages {

struct message_base {
  message_header header;
};

} // namespace mori_echo::messages
