#pragma once

#include <boost/asio/io_context.hpp>
#include <cstdint>

namespace mori_echo {

struct echo_server_context {
  boost::asio::io_context& io_context;
  std::uint16_t port;
};

} // namespace mori_echo
