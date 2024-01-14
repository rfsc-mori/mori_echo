#pragma once

#include "client_authenticator/client_authenticator.hpp"

#include <memory>
#include <string_view>

namespace mori_echo::auth {

class test_client_authenticator final : public client_authenticator {
private:
  test_client_authenticator() = default;

public:
  [[nodiscard]] static auto create() -> std::shared_ptr<client_authenticator> {
    return std::shared_ptr<test_client_authenticator>{
        new test_client_authenticator};
  }

  auto authenticate(std::string_view username, std::string_view password)
      -> void override final;
};

} // namespace mori_echo::auth
