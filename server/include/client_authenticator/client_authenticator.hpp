#pragma once

#include <string_view>

namespace mori_echo::auth {

class client_authenticator {
public:
  virtual auto authenticate(std::string_view username,
                            std::string_view password) -> void = 0;
};

} // namespace mori_echo::auth
