#pragma once

#include <cstdint>

namespace mori_echo::mori_status {

enum class login_status : std::uint16_t { FAILED = 0, OK = 1 };

} // namespace mori_echo::mori_status
