#pragma once

#include <boost/asio/io_context.hpp>
#include <cstdint>

#include "client_authenticator/client_authenticator.hpp"

namespace mori_echo {

struct echo_server_context {
  boost::asio::io_context& io_context;
  std::uint16_t port;

  std::shared_ptr<auth::client_authenticator> authenticator;
};

} // namespace mori_echo
