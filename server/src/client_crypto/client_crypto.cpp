#include "client_crypto/client_crypto.hpp"

#include <numeric>

namespace mori_echo::crypto {

auto calculate_checksum(std::string_view data) -> std::uint8_t {
  return std::accumulate(
      data.begin(), data.end(), std::uint8_t{0},
      [](std::uint8_t checksum, decltype(data)::value_type each) {
        return checksum + static_cast<std::uint8_t>(each);
      });
}

auto calculate_initial_key(crypto_message_params args) -> std::uint32_t {
  return (args.sequence << 16) | (args.username_sum << 8) | args.password_sum;
}

auto calculate_next_key(std::uint32_t key) -> std::uint32_t {
  return (key * 1103515245 + 12345) % 0x7FFFFFFF;
}

auto decrypt(crypto_message_params args, std::vector<std::byte> message)
    -> std::vector<std::byte> {
  auto key = calculate_initial_key(std::move(args));

  for (auto& each : message) {
    key = calculate_next_key(key);

    const auto cipher_key = static_cast<std::uint8_t>(key % 256);

    const auto decrypted =
        static_cast<std::uint8_t>(static_cast<std::uint8_t>(each) ^ cipher_key);

    each = static_cast<std::byte>(decrypted);
  }

  return message;
}

} // namespace mori_echo::crypto
