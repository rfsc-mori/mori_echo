#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "crypto_message_params.hpp"

namespace mori_echo::crypto {

auto calculate_checksum(std::string_view message) -> std::uint8_t;

auto decrypt(crypto_message_params args, std::vector<std::byte> message)
    -> std::vector<std::byte>;

} // namespace mori_echo::crypto
