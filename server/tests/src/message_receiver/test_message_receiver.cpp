#include "message_receiver/message_receiver.hpp"

#include <boost/endian/conversion.hpp>
#include <cstdint>
#include <limits>

#include "exceptions/server_error.hpp"
#include "message_types/echo_response.hpp"
#include "message_types/login_response.hpp"
#include "mori_echo/server_config.hpp"

namespace mori_echo {

static_assert(config::byte_order == config::endian_mode::LITTLE_ENDIAN_MODE ||
                  config::byte_order == config::endian_mode::BIG_ENDIAN_MODE,
              "Invalid byte order configuration.");

inline constexpr auto header_size =
    sizeof(std::uint16_t) + sizeof(std::uint8_t) + sizeof(std::uint8_t);

template <>
auto receive_message<messages::login_response>(client_channel& channel,
                                               messages::message_header header)
    -> boost::asio::awaitable<messages::login_response> {
  constexpr auto min_message_size = header_size + sizeof(std::uint16_t);

  constexpr auto max_message_size = min_message_size;

  if (header.type != messages::message_type::LOGIN_RESPONSE) {
    throw exceptions::server_error{"Wrong message type."};
  }

  if (header.total_size < min_message_size) {
    throw exceptions::server_error{"Message too short."};
  } else if (header.total_size > max_message_size) {
    throw exceptions::server_error{"Message too long."};
  }

  auto status_code = co_await channel.receive_as<std::uint16_t>();

  if constexpr (config::byte_order == config::endian_mode::LITTLE_ENDIAN_MODE) {
    boost::endian::little_to_native_inplace(status_code);
  } else {
    boost::endian::big_to_native_inplace(status_code);
  }

  auto actual_status_code = mori_status::login_status{};

  switch (status_code) {
    case static_cast<std::uint16_t>(mori_status::login_status::OK):
    case static_cast<std::uint16_t>(mori_status::login_status::FAILED):
      actual_status_code = static_cast<mori_status::login_status>(status_code);
      break;

    default:
      throw exceptions::server_error{"Invalid login status code."};
  }

  auto message = messages::login_response{};

  message.header = std::move(header);
  message.status_code = actual_status_code;

  co_return message;
}

template <>
auto receive_message<messages::echo_response>(client_channel& channel,
                                              messages::message_header header)
    -> boost::asio::awaitable<messages::echo_response> {
  constexpr auto min_message_size = header_size + sizeof(std::uint16_t);

  constexpr auto max_message_size = std::min(
      std::size_t{
          std::numeric_limits<decltype(std::declval<messages::message_header>()
                                           .total_size)>::max()},
      min_message_size + std::numeric_limits<std::uint16_t>::max());

  if (header.type != messages::message_type::ECHO_RESPONSE) {
    throw exceptions::server_error{"Wrong message type."};
  }

  if (header.total_size < min_message_size) {
    throw exceptions::server_error{"Message too short."};
  } else if (header.total_size > max_message_size) {
    throw exceptions::server_error{"Message too long."};
  }

  auto message_size = co_await channel.receive_as<std::uint16_t>();

  if constexpr (config::byte_order == config::endian_mode::LITTLE_ENDIAN_MODE) {
    boost::endian::little_to_native_inplace(message_size);
  } else {
    boost::endian::big_to_native_inplace(message_size);
  }

  const auto final_message_size =
      static_cast<std::uint16_t>(min_message_size + message_size);

  if (header.total_size != final_message_size) {
    throw exceptions::server_error{"Message size mismatch."};
  }

  auto plain_message = co_await channel.receive(message_size);

  assert(plain_message.size() == message_size);

  auto message = messages::echo_response{};

  message.header = std::move(header);
  message.message_size = message_size;
  message.plain_message = std::move(plain_message);

  co_return message;
}

} // namespace mori_echo
