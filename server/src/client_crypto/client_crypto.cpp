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

auto calculate_initial_key(std::uint8_t username_sum, std::uint8_t password_sum,
                           std::uint8_t sequence) -> std::uint32_t {
  return (sequence << 16) | (username_sum << 8) | password_sum;
}

} // namespace mori_echo::crypto
