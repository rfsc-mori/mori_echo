# MoriEcho

A TCP Echo Server with authentication and encryption support.

## Server execution flow:

1. A new client connects via TCP/IPv4 to port `31216` and is assigned a session.
2. The client should send a `login_request` message with any username and password combination.
3. The server will reply with a `login_response` with status `OK`.
4. The client can now send `echo_request` messages with an encrypted `cipher_message`.
5. The server will reply with `echo_response` messages, containing the decrypted `plain_message`.
6. The client can disconnect at any time, and their session will destroyed.
7. To close the server from a terminal, please send a `SIGINT` (Ctrl+C).

For more details about the cipher algorithm, please check [client_crypto.cpp](server/src/client_crypto/client_crypto.cpp) and [the cipher test](server/tests/src/cipher.cpp).

For a reference implementation of a client software, please check the [business rules test](server/tests/src/business_rules.cpp).

### Concurrency

A single thread is spawned to run the client coroutine tasks concurrently.

This supports multiple concurrent clients with IO multiplexing.

For further details, please check the [concurrency test](server/tests/src/concurrency.cpp), which runs a complete business rule test over 500 simultaneous connections.

### Parallelism

It is possible to implement parallelism in combination with concurrency by using a `thread_pool` instead of `io_context` in [main.cpp](server/src/main.cpp).

## Static configuration:

You can edit the [server_config.hpp](include/mori_echo/server_config.hpp) to change build-time configurations.

### Endianness

The default byte order for this network protocol w.r.t. binary serialization and deserialization of integers of size higher than 1 is assumed to be little endian.

To change this behavior please set `byte_order` to either `LITTLE_ENDIAN_MODE` or `BIG_ENDIAN_MODE`.

## Core dependencies:

- C++20 and a compiler that implements [P0912R5](https://wg21.link/P0912R5)
- Boost.Asio with coroutines support
- Boost.Endian to handle protocol endianness
- Boost.Uuid to identify client sessions
- Boost.Test to test business rules and concurrency
- spdlog to provide detailed logging

## Development environment:

- [x] Uses my personal DevContainer template for VSCode
- [x] Provides Docker environment for building, testing and running

### DevContainer:

- Open this project in VSCode while having the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) installed.
- Reopen the folder in container using the extension.

### Manual:

Build dependencies:

- GCC >=12 (build-essential)
- CMake >=3.25
- Ninja
- vcpkg ($VCPKG_ROOT must be set)

## Building

### Manually:

#### Debug:

```sh
cmake -B build/ --preset debug
cmake --build build/
```

#### Release:

```sh
cmake -B build/ --preset release
cmake --build build/
```

## Running:

### Locally:

```sh
./build/server/mori_echo_server
```

### Docker, locally:

```sh
docker compose up
```

### Docker, from GitHub Registry:

#### Little-Endian version:

```sh
docker run --rm -p 31216:31216 ghcr.io/rfsc-mori/mori_echo:little_endian
```

#### Big-Endian version:

```sh
docker run --rm -p 31216:31216 ghcr.io/rfsc-mori/mori_echo:big_endian
```

# Testing

If you decided to use `docker compose up`, the tests are already executed during the build process.

## Everything:

```sh
ctest --preset tests
```

### Only business rules:

```sh
ctest --preset tests -R business_rules
```

### Only concurrency tests:

```sh
ctest --preset tests -R concurrency
```

### Only cipher tests:

```sh
ctest --preset tests -R cipher
```

## Server overview:

- [x] Provides a TCP server capable of asynchronous processing
- [x] Validates messages as efficiently as possible
- [x] Rejects invalid messages and attempts to fail fast
- [x] Requires user authentication before echoing
- [x] Accepts any combination of username and password
- [x] Keeps a per-session user context, containing the username and password checksums
- [x] Echoes the message from the request
- [x] Decrypts client messages

## Messages format:

### Header

```cpp
enum class message_type : uint8_t {
    LOGIN_REQUEST = 0,
    LOGIN_RESPONSE = 1,
    ECHO_REQUEST = 2,
    ECHO_RESPONSE = 3
};

struct message_header {
    uint16_t total_size;
    message_type type;
    uint8_t sequence;
};
```

### Login Request

```cpp
inline constexpr auto username_size = 32;
inline constexpr auto password_size = 32;

struct login_request {
    message_header header; // .message_type = LOGIN_REQUEST (0)
    fixed_length_container<char, message_limits::username_size> username;
    fixed_length_container<char, message_limits::password_size> password;
};
```

### Login Response

```cpp
enum class login_status : uint16_t {
    FAILED = 0,
    OK = 1
};

struct login_response {
    message_header header; // .message_type = LOGIN_RESPONSE (1)
    login_status status_code;
};
```

### Echo Request

```cpp
struct echo_request {
    message_header header; // .message_type = ECHO_REQUEST (2)
    uint16_t message_size;
    fixed_length_container<char, message_size> cipher_message;
};
```

### Echo Response

```cpp
struct echo_response {
    message_header header; // .message_type = ECHO_RESPONSE (3)
    uint16_t message_size;
    fixed_length_container<char, message_size> plain_message;
};
```

# Attributions

This project uses Microsoft's CPP DevContainer image for the development environment:  
https://github.com/devcontainers/images/blob/main/src/cpp/README.md

This project uses the spdlog logging library:  
https://github.com/gabime/spdlog/blob/v1.x/LICENSE

This project uses libraries from Boost:  
https://www.boost.org/LICENSE_1_0.txt

This project uses vcpkg to install dependencies:  
https://github.com/microsoft/vcpkg/blob/master/LICENSE.txt

# License

MIT

# Author

Rafael Fillipe Silva (https://github.com/rfsc-mori)
