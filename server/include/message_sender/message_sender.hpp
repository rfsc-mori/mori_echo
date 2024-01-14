#pragma once

#include <boost/asio/awaitable.hpp>

#include "client_channel/client_channel.hpp"
#include "message_types/echo_response.hpp"
#include "message_types/login_response.hpp"
#include "message_types/message_base.hpp"
#include "mori_status/login_status.hpp"

namespace mori_echo {

template <messages::MoriEchoMessage T> struct send_message;

template <> struct send_message<messages::login_response> {
  auto operator()(client_channel& channel, std::uint8_t sequence,
                  mori_status::login_status status_code)
      -> boost::asio::awaitable<void>;
};

template <> struct send_message<messages::echo_response> {
  auto operator()(client_channel& channel, std::uint8_t sequence,
                  const std::vector<std::byte>& message)
      -> boost::asio::awaitable<void>;
};

} // namespace mori_echo
