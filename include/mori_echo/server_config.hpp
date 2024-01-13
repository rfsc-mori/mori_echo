#pragma once

#include <cstdint>

namespace mori_echo::config {

inline constexpr auto tcp_port = std::uint16_t{31216};

inline constexpr auto username_size = std::size_t{32};
inline constexpr auto password_size = std::size_t{32};

} // namespace mori_echo::config
