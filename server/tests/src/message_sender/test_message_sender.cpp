#include "test_message_sender.hpp"

#include <boost/endian/conversion.hpp>
#include <cstdint>

#include "exceptions/client_error.hpp"
#include "message_types/echo_request.hpp"
#include "message_types/login_request.hpp"
#include "mori_echo/server_config.hpp"

namespace mori_echo {

static_assert(config::byte_order == config::endian_mode::LITTLE_ENDIAN_MODE ||
                  config::byte_order == config::endian_mode::BIG_ENDIAN_MODE,
              "Invalid byte order configuration.");

inline constexpr auto header_size =
    sizeof(std::uint16_t) + sizeof(std::uint8_t) + sizeof(std::uint8_t);

[[nodiscard]] auto
send_header(client_channel& channel, std::uint16_t total_size,
            messages::message_type type, std::uint8_t sequence)
    -> boost::asio::awaitable<void>;

auto send_message<messages::login_request>::operator()(
    client_channel& channel, std::uint8_t sequence, std::string_view username,
    std::string_view password) -> boost::asio::awaitable<void> {
  auto total_size = std::uint16_t{header_size + config::username_size +
                                  config::password_size};

  if constexpr (config::byte_order == config::endian_mode::LITTLE_ENDIAN_MODE) {
    boost::endian::native_to_little_inplace(total_size);
  } else {
    boost::endian::native_to_big_inplace(total_size);
  }

  if (username.size() >= config::username_size) {
    throw exceptions::client_error{"Username too long."};
  }

  if (password.size() >= config::username_size) {
    throw exceptions::client_error{"Password too long."};
  }

  co_await send_header(channel, total_size,
                       messages::message_type::LOGIN_REQUEST, sequence);

  auto username_data = std::vector<std::byte>{config::username_size};
  auto password_data = std::vector<std::byte>{config::password_size};

  std::transform(username.begin(), username.end(), username_data.begin(),
                 [](char each) { return static_cast<std::byte>(each); });

  std::transform(password.begin(), password.end(), password_data.begin(),
                 [](char each) { return static_cast<std::byte>(each); });

  co_await channel.send(username_data);
  co_await channel.send(password_data);
}

auto send_message<messages::echo_request>::operator()(
    client_channel& channel, std::uint8_t sequence,
    const std::vector<std::byte>& message) -> boost::asio::awaitable<void> {
  constexpr auto max_message_size = std::numeric_limits<std::uint16_t>::max() -
                                    sizeof(std::uint16_t) - header_size;

  if (message.size() > max_message_size) {
    throw exceptions::client_error{"Message too long."};
  }

  auto total_size = static_cast<std::uint16_t>(
      header_size + sizeof(std::uint16_t) + message.size());

  if constexpr (config::byte_order == config::endian_mode::LITTLE_ENDIAN_MODE) {
    boost::endian::native_to_little_inplace(total_size);
  } else {
    boost::endian::native_to_big_inplace(total_size);
  }

  co_await send_header(channel, total_size,
                       messages::message_type::ECHO_REQUEST, sequence);

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
