#pragma once

#include <boost/asio/awaitable.hpp>
#include <memory>

#include "client_channel/client_channel.hpp"
#include "message_types/message_base.hpp"

namespace mori_echo {

[[nodiscard]] auto receive_message(client_channel& channel)
    -> boost::asio::awaitable<std::unique_ptr<messages::message_base>>;

} // namespace mori_echo
