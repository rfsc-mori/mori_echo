# MoriEcho

This is a TCP Echo Server assignment.

## Main dependencies:

- C++20
- boost.asio with coroutines support

## Development environment:

- [x] Uses my personal DevContainer template for VSCode
- [ ] Provides Docker environments for building, testing and running

## Building

### Debug:

```sh
cmake -B build/ --preset debug
cmake --build build/
```

### Release:

```sh
cmake -B build/ --preset release
cmake --build build/
```

## Server overview:

- [x] Provides a TCP server capable of asynchronous processing
- [ ] Validates messages as efficiently as possible
- [ ] Rejects invalid messages and attempts to fail fast
- [ ] Requires user authentication before echoing
- [ ] Accepts any combination of username and password
- [ ] Keeps a per-session user context, containing the current cipher_key
- [ ] Decrypts client messages
- [ ] Echoes the plain (decrypted) message from the echo request

## Reference client implementation:

- [ ] Accepts a command sequence from execution arguments
- [ ] Authenticates using the parameters from execution arguments
- [ ] Logs all sent and received messages
- [ ] Validates the received messages
- [ ] Exits with return code 1 if validation fails
- [ ] Execute all specified commands, sequentially
- [ ] Exits with return code 0 if no errors occurs

### Command line example:

```
./build/reference_client -u test -p test first_message second_message
```

## Messages format:

### [x] Header

```cpp
namespace mori_echo::messages {
    enum class message_types : uint8_t {
        LOGIN_REQUEST = 0,
        LOGIN_RESPONSE = 1,
        ECHO_REQUEST = 2,
        ECHO_RESPONSE = 3
    };

    struct message_header {
        uint16_t total_message_size;
        message_types message_type;
        uint8_t message_sequence;
    };
}
```

### [x] Login Request

```cpp
namespace mori_echo::message_limits {
    inline constexpr auto username_size = 32;
    inline constexpr auto password_size = 32;
}

namespace mori_echo::messages {
    struct login_request {
        message_header header; // .message_type = LOGIN_REQUEST (0)
        std::array<char, message_limits::username_size> username;
        std::array<char, message_limits::password_size> password;
    };
}
```

### [x] Login Response

```cpp
namespace mori_echo::mori_status {
    enum class login_status : uint32_t {
        FAILED = 0,
        OK = 1
    };
}

namespace mori_echo::messages {
    struct login_response {
        message_header header; // .message_type = LOGIN_RESPONSE (1)
        mori_status::login_status status_code;
    };
}
```

### [x] Echo Request

```cpp
namespace mori_echo::messages {
    struct echo_request {
        message_header header; // .message_type = ECHO_REQUEST (2)
        uint16_t message_size;
        fixed_length_container<char, message_size> cipher_message;
    };
}
```

### [x] Echo Response

```cpp
namespace mori_echo::messages {
    struct echo_response {
        message_header header; // .message_type = ECHO_RESPONSE (3)
        uint16_t message_size;
        fixed_length_container<char, message_size> plain_message;
    };
}
```

# Attributions

This project uses Microsoft's CPP DevContainer image for the development environment:  
https://github.com/devcontainers/images/blob/main/src/cpp/README.md

# License

MIT

# Author

Rafael Fillipe Silva (https://github.com/rfsc-mori)
