#include "client_channel/client_channel.hpp"

#include <boost/asio/read.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>

namespace mori_echo {

auto client_channel::receive(std::size_t count)
    -> boost::asio::awaitable<std::vector<std::byte>> {
  auto buffer = std::vector<std::byte>(count, {});

  co_await boost::asio::async_read(socket, boost::asio::buffer(buffer),
                                   boost::asio::use_awaitable);

  co_return buffer;
}

auto client_channel::send(const std::vector<std::byte>& data)
    -> boost::asio::awaitable<void> {
  co_await boost::asio::async_write(socket, boost::asio::buffer(data),
                                    boost::asio::use_awaitable);
}

auto client_channel::receive_raw(void* buffer, std::size_t size)
    -> boost::asio::awaitable<void> {
  assert(buffer != nullptr);

  co_await boost::asio::async_read(socket, boost::asio::buffer(buffer, size),
                                   boost::asio::use_awaitable);
}

auto client_channel::send_raw(void* buffer, std::size_t size)
    -> boost::asio::awaitable<void> {
  assert(buffer != nullptr);

  co_await boost::asio::async_write(socket, boost::asio::buffer(buffer, size),
                                    boost::asio::use_awaitable);
}

} // namespace mori_echo
