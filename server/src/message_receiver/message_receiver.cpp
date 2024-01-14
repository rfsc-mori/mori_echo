#include "message_receiver/message_receiver.hpp"

#include <boost/endian/conversion.hpp>
#include <cstdint>
#include <limits>

#include "exceptions/client_error.hpp"
#include "message_types/echo_request.hpp"
#include "message_types/login_request.hpp"
#include "message_types/message_header.hpp"
#include "mori_echo/server_config.hpp"

namespace mori_echo {

static_assert(config::byte_order == config::endian_mode::LITTLE_ENDIAN_MODE ||
                  config::byte_order == config::endian_mode::BIG_ENDIAN_MODE,
              "Invalid byte order configuration.");

inline constexpr auto header_size =
    sizeof(std::uint16_t) + sizeof(std::uint8_t) + sizeof(std::uint8_t);

[[nodiscard]] auto receive_header(client_channel& channel)
    -> boost::asio::awaitable<messages::message_header> {
  constexpr auto min_message_size = header_size + 1;

  constexpr auto max_message_size = std::min(
      std::size_t{
          std::numeric_limits<decltype(std::declval<messages::message_header>()
                                           .total_size)>::max()},
      header_size + sizeof(std::uint16_t) +
          std::numeric_limits<std::uint16_t>::max());

  auto total_size = co_await channel.receive_as<std::uint16_t>();

  if constexpr (config::byte_order == config::endian_mode::LITTLE_ENDIAN_MODE) {
    boost::endian::little_to_native_inplace(total_size);
  } else {
    boost::endian::big_to_native_inplace(total_size);
  }

  if (total_size < min_message_size) {
    throw exceptions::client_error{"Message too short."};
  } else if (total_size > max_message_size) {
    throw exceptions::client_error{"Message too long."};
  }

  const auto type = co_await channel.receive_as<std::uint8_t>();
  const auto sequence = co_await channel.receive_as<std::uint8_t>();

  auto actual_type = messages::message_type{};

  switch (type) {
    case static_cast<std::uint8_t>(messages::message_type::LOGIN_REQUEST):
    case static_cast<std::uint8_t>(messages::message_type::LOGIN_RESPONSE):
    case static_cast<std::uint8_t>(messages::message_type::ECHO_REQUEST):
    case static_cast<std::uint8_t>(messages::message_type::ECHO_RESPONSE):
      actual_type = static_cast<messages::message_type>(type);
      break;

    default:
      throw exceptions::client_error{"Invalid message type."};
  }

  co_return messages::message_header{
      .total_size = total_size,
      .type = actual_type,
      .sequence = sequence,
  };
}

template <>
auto receive_message<messages::login_request>(client_channel& channel,
                                              messages::message_header header)
    -> boost::asio::awaitable<messages::login_request> {
  constexpr auto min_message_size =
      header_size + config::username_size + config::password_size;

  constexpr auto max_message_size = min_message_size;

  if (header.type != messages::message_type::LOGIN_REQUEST) {
    throw exceptions::client_error{"Wrong message type."};
  }

  if (header.total_size < min_message_size) {
    throw exceptions::client_error{"Message too short."};
  } else if (header.total_size > max_message_size) {
    throw exceptions::client_error{"Message too long."};
  }

  auto username = co_await channel.receive(config::username_size);
  auto password = co_await channel.receive(config::password_size);

  assert(username.size() == config::username_size);
  assert(password.size() == config::password_size);

  static_assert(config::username_size > 0);
  static_assert(config::password_size > 0);

  // For ASCIIZ of size X, the max length is X - 1. Force null-terminator.
  username[config::username_size - 1] = std::byte{'\0'};
  password[config::password_size - 1] = std::byte{'\0'};

  auto message = messages::login_request{};

  message.header = std::move(header);

  message.username =
      std::string{reinterpret_cast<const char*>(username.data())};

  message.password =
      std::string{reinterpret_cast<const char*>(password.data())};

  co_return message;
}

template <>
auto receive_message<messages::echo_request>(client_channel& channel,
                                             messages::message_header header)
    -> boost::asio::awaitable<messages::echo_request> {
  constexpr auto min_message_size = header_size + sizeof(std::uint16_t);

  constexpr auto max_message_size = std::min(
      std::size_t{
          std::numeric_limits<decltype(std::declval<messages::message_header>()
                                           .total_size)>::max()},
      min_message_size + std::numeric_limits<std::uint16_t>::max());

  if (header.type != messages::message_type::ECHO_REQUEST) {
    throw exceptions::client_error{"Wrong message type."};
  }

  if (header.total_size < min_message_size) {
    throw exceptions::client_error{"Message too short."};
  } else if (header.total_size > max_message_size) {
    throw exceptions::client_error{"Message too long."};
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
    throw exceptions::client_error{"Message size mismatch."};
  }

  auto cipher_message = co_await channel.receive(message_size);

  assert(cipher_message.size() == message_size);

  auto message = messages::echo_request{};

  message.header = std::move(header);
  message.message_size = message_size;
  message.cipher_message = std::move(cipher_message);

  co_return message;
}

} // namespace mori_echo
