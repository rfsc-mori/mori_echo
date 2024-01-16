#pragma once

#include "client_crypto/client_crypto.hpp"

namespace mori_echo::crypto {

auto calculate_initial_key(crypto_message_params args) -> std::uint32_t;

auto calculate_next_key(std::uint32_t key) -> std::uint32_t;

auto calculate_cipher_key(std::uint8_t key) -> std::uint8_t;

auto encrypt(crypto_message_params args, std::vector<std::byte> message)
    -> std::vector<std::byte>;

} // namespace mori_echo::crypto
