#pragma once

#include <boost/asio/any_io_executor.hpp>

#include "echo_server_config.hpp"

namespace mori_echo {

auto spawn_server(boost::asio::any_io_executor executor, echo_server_config cfg)
    -> void;

} // namespace mori_echo
