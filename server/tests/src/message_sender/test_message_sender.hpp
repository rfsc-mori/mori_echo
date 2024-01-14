#pragma once

#include "message_sender/message_sender.hpp"
#include "message_types/echo_request.hpp"
#include "message_types/login_request.hpp"

namespace mori_echo {

template <> struct send_message<messages::login_request> {
  auto operator()(client_channel& channel, std::uint8_t sequence,
                  std::string_view username, std::string_view password)
      -> boost::asio::awaitable<void>;
};

template <> struct send_message<messages::echo_request> {
  auto operator()(client_channel& channel, std::uint8_t sequence,
                  const std::vector<std::byte>& message)
      -> boost::asio::awaitable<void>;
};

} // namespace mori_echo
