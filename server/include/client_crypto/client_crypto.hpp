#pragma once

#include <cstdint>
#include <string_view>

namespace mori_echo::crypto {

auto calculate_checksum(std::string_view message) -> std::uint8_t;

auto calculate_initial_key(std::uint8_t username_sum, std::uint8_t password_sum,
                           std::uint8_t sequence) -> std::uint32_t;

} // namespace mori_echo::crypto
