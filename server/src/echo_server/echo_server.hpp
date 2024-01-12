#pragma once

#include <boost/asio/io_context.hpp>

namespace mori_echo {

auto spawn_server(boost::asio::io_context& context, std::uint16_t port) -> void;

} // namespace mori_echo
