#pragma once

#include "message_base.hpp"
#include "mori_status/login_status.hpp"

namespace mori_echo::messages {

struct login_response : public message_base {
  mori_status::login_status status_code = {};
};

} // namespace mori_echo::messages
