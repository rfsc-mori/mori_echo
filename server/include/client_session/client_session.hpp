#pragma once

#include <string>

namespace mori_echo {

struct client_session {
  std::string uuid;
  std::string address;

  bool is_logged_in = false;
};

} // namespace mori_echo
