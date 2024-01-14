#include "message_sender/message_sender.hpp"

#include <boost/endian/conversion.hpp>
#include <cstdint>

#include "exceptions/server_error.hpp"
#include "message_types/echo_response.hpp"
#include "message_types/login_response.hpp"
#include "mori_echo/server_config.hpp"
#include "mori_status/login_status.hpp"

namespace mori_echo {

static_assert(config::byte_order == config::endian_mode::LITTLE_ENDIAN_MODE ||
                  config::byte_order == config::endian_mode::BIG_ENDIAN_MODE,
              "Invalid byte order configuration.");

inline constexpr auto header_size =
    sizeof(std::uint16_t) + sizeof(std::uint8_t) + sizeof(std::uint8_t);

[[nodiscard]] auto
send_header(client_channel& channel, std::uint16_t total_size,
            messages::message_type type, std::uint8_t sequence)
    -> boost::asio::awaitable<void> {
  co_await channel.send_as(total_size);

  co_await channel.send_as(
      static_cast<std::underlying_type_t<decltype(type)>>(type));

  co_await channel.send_as(sequence);
}

auto send_message<messages::login_response>::operator()(
    client_channel& channel, std::uint8_t sequence,
    mori_status::login_status status_code) -> boost::asio::awaitable<void> {
  auto total_size = std::uint16_t{header_size + sizeof(status_code)};

  if constexpr (config::byte_order == config::endian_mode::LITTLE_ENDIAN_MODE) {
    boost::endian::native_to_little_inplace(total_size);
  } else {
    boost::endian::native_to_big_inplace(total_size);
  }

  co_await send_header(channel, total_size,
                       messages::message_type::LOGIN_RESPONSE, sequence);

  const auto login_status_code =
      static_cast<std::underlying_type_t<decltype(status_code)>>(status_code);

  if constexpr (config::byte_order == config::endian_mode::LITTLE_ENDIAN_MODE) {
    boost::endian::native_to_little_inplace(login_status_code);
  } else {
    boost::endian::native_to_big_inplace(login_status_code);
  }

  co_await channel.send_as(login_status_code);
}

auto send_message<messages::echo_response>::operator()(
    client_channel& channel, std::uint8_t sequence,
    const std::vector<std::byte>& message) -> boost::asio::awaitable<void> {
  constexpr auto max_message_size = std::numeric_limits<std::uint16_t>::max() -
                                    sizeof(std::uint16_t) - header_size;

  if (message.size() > max_message_size) {
    throw exceptions::server_error{"Message too long."};
  }

  auto total_size = static_cast<std::uint16_t>(
      header_size + sizeof(std::uint16_t) + message.size());

  if constexpr (config::byte_order == config::endian_mode::LITTLE_ENDIAN_MODE) {
    boost::endian::native_to_little_inplace(total_size);
  } else {
    boost::endian::native_to_big_inplace(total_size);
  }

  co_await send_header(channel, total_size,
                       messages::message_type::ECHO_RESPONSE, sequence);

  auto message_size = static_cast<std::uint16_t>(message.size());

  if constexpr (config::byte_order == config::endian_mode::LITTLE_ENDIAN_MODE) {
    boost::endian::native_to_little_inplace(message_size);
  } else {
    boost::endian::native_to_big_inplace(message_size);
  }

  co_await channel.send_as(message_size);

  co_await channel.send(message);
}

} // namespace mori_echo
