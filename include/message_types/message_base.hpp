#pragma once

#include "message_header.hpp"

#include <concepts>

namespace mori_echo::messages {

struct message_base {
  message_header header;
};

template <typename T>
concept MoriEchoMessage = std::derived_from<T, message_base>;

} // namespace mori_echo::messages
