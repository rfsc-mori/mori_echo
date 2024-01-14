#pragma once

#include <cstdint>

#include "endian_mode.hpp"

namespace mori_echo::config {

inline constexpr auto tcp_port = std::uint16_t{31216};

inline constexpr auto byte_order = endian_mode::LITTLE_ENDIAN_MODE;

inline constexpr auto username_size = std::size_t{32};
inline constexpr auto password_size = std::size_t{32};

inline constexpr auto enable_decryption = true;

} // namespace mori_echo::config
