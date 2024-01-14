#pragma once

#include "client_crypto/client_crypto.hpp"

namespace mori_echo::crypto {

auto encrypt(crypto_message_params args, std::vector<std::byte> message)
    -> std::vector<std::byte>;

} // namespace mori_echo::crypto
