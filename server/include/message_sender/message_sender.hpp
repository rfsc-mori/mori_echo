#pragma once

#include <boost/asio/awaitable.hpp>

#include "client_channel/client_channel.hpp"
#include "message_types/message_base.hpp"

namespace mori_echo {

template <messages::MoriEchoMessage T>
[[nodiscard]] auto send_message(client_channel& channel, std::uint8_t sequence,
                                auto... message_args)
    -> boost::asio::awaitable<void>;

} // namespace mori_echo
