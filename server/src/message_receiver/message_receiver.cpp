#include "message_receiver/message_receiver.hpp"

#include <boost/endian/conversion.hpp>

#include "exceptions/client_error.hpp"
#include "message_types/echo_request.hpp"
#include "message_types/login_request.hpp"
#include "message_types/message_header.hpp"
#include "mori_echo/server_config.hpp"

namespace mori_echo {

static_assert(config::byte_order == config::endian_mode::LITTLE_ENDIAN_MODE ||
                  config::byte_order == config::endian_mode::BIG_ENDIAN_MODE,
              "Invalid byte order configuration.");

[[nodiscard]] auto receive_header(client_channel& channel)
    -> boost::asio::awaitable<messages::message_header> {
  auto total_size = co_await channel.receive_as<std::uint16_t>();
  const auto type = co_await channel.receive_as<std::uint8_t>();
  const auto sequence = co_await channel.receive_as<std::uint8_t>();

  if constexpr (config::byte_order == config::endian_mode::LITTLE_ENDIAN_MODE) {
    total_size = boost::endian::little_to_native(total_size);
  } else {
    total_size = boost::endian::big_to_native(total_size);
  }

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

[[nodiscard]] auto receive_login_request(client_channel& channel,
                                         messages::message_header header)
    -> boost::asio::awaitable<std::unique_ptr<messages::login_request>> {
  const auto username = co_await channel.receive(config::username_size);
  const auto password = co_await channel.receive(config::password_size);

  assert(username.size() == config::username_size);
  assert(password.size() == config::password_size);

  auto message = std::make_unique<messages::login_request>();

  message->header = std::move(header);
  std::copy(username.begin(), username.end(), message->username.begin());
  std::copy(password.begin(), password.end(), message->password.begin());

  co_return message;
}

[[nodiscard]] auto receive_echo_request(client_channel& channel,
                                        messages::message_header header)
    -> boost::asio::awaitable<std::unique_ptr<messages::echo_request>> {
  auto message_size = co_await channel.receive_as<std::uint16_t>();

  if constexpr (config::byte_order == config::endian_mode::LITTLE_ENDIAN_MODE) {
    message_size = boost::endian::little_to_native(message_size);
  } else {
    message_size = boost::endian::big_to_native(message_size);
  }

  const auto cipher_message = co_await channel.receive(message_size);

  assert(cipher_message.size() == message_size);

  auto message = std::make_unique<messages::echo_request>();

  message->header = std::move(header);
  message->message_size = message_size;
  std::copy(cipher_message.begin(), cipher_message.end(),
            message->cipher_message.begin());

  co_return message;
}

auto receive_message(client_channel& channel)
    -> boost::asio::awaitable<std::unique_ptr<messages::message_base>> {
  auto header = co_await receive_header(channel);

  switch (header.type) {
    case messages::message_type::LOGIN_REQUEST:
      co_return co_await receive_login_request(channel, std::move(header));

    case messages::message_type::ECHO_REQUEST:
      co_return co_await receive_echo_request(channel, std::move(header));

    case messages::message_type::LOGIN_RESPONSE:
    case messages::message_type::ECHO_RESPONSE:
      throw exceptions::client_error{"Client should never send this message."};
  }

  throw exceptions::client_error{"Invalid message type."};
}

} // namespace mori_echo
