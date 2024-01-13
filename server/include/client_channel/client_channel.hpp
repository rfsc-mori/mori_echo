#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <vector>

namespace mori_echo {

class [[nodiscard]] client_channel {
public:
  client_channel(boost::asio::ip::tcp::socket client_socket)
      : socket{std::move(client_socket)} {}

  [[nodiscard]] auto receive(std::size_t count)
      -> boost::asio::awaitable<std::vector<std::byte>>;

  [[nodiscard]] auto send(std::vector<std::byte> data)
      -> boost::asio::awaitable<void>;

  template <typename T>
    requires std::is_trivially_copyable_v<T>
  [[nodiscard]] auto receive_as() -> boost::asio::awaitable<T> {
    auto buffer = T{};
    co_await receive_raw(&buffer, sizeof(T));
    co_return buffer;
  }

private:
  [[nodiscard]] auto receive_raw(void* buffer, std::size_t size)
  auto receive_into(void* buffer, std::size_t size)
      -> boost::asio::awaitable<void>;

private:
  boost::asio::ip::tcp::socket socket;
};

} // namespace mori_echo
