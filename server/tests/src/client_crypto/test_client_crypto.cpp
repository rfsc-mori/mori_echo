#include "test_client_crypto.hpp"

namespace mori_echo::crypto {

auto encrypt(crypto_message_params args, std::vector<std::byte> message)
    -> std::vector<std::byte> {
  return decrypt(args, message);
}

} // namespace mori_echo::crypto
