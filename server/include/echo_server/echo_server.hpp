#pragma once

#include "echo_server_context.hpp"

namespace mori_echo {

auto spawn_server(echo_server_context ctx) -> void;

} // namespace mori_echo
