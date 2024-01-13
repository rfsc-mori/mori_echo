#pragma once

#include <stdexcept>

namespace mori_echo::exceptions {

class client_error : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

} // namespace mori_echo::exceptions
