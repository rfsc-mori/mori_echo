#pragma once

#include <cstdint>

namespace mori_echo::crypto {

struct crypto_message_params {
  std::uint8_t username_sum;
  std::uint8_t password_sum;
  std::uint8_t sequence;
};

} // namespace mori_echo::crypto
