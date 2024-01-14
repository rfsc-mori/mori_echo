#pragma once

#include "echo_server_config.hpp"

namespace mori_echo {

auto spawn_server(boost::asio::io_context& io_context, echo_server_config cfg)
    -> void;

} // namespace mori_echo
