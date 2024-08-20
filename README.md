# Cpp-SocketLibrary

A ServerSocket and Socket library for Windows and Linux aiming to support both Wifi and Bluetooth communication.

## Getting Started

### Dependencies

- [CMake](https://cmake.org/download/) and `make`

The following **linux** dependencies are required:
- `libbluetooth-dev`
- `libglib2.0-dev`
- `bluez`

### Building the Library and Running the Tests - Linux

1. To build the library, firstly run cmake: `cmake . -B build-linux` in the root directory of the repository (`CppSocketLibrary/`).
2. Then move into the new `build-linux` folder: `cd build-linux`.
3. Then you can run `make` to build the library.
4. Then you can run `make check` to run the available tests.

### Building the Library and Running the Tests - Windows

1. To build the library, firstly run cmake: `cmake . -B build` in the root directory of the repository (`CppSocketLibrary/`).
2. This will create a `.sln` file which can be opened in `Visual Studio`.
3. Once opened (and with the appropriate windows service packs installed - `MSVC v143 - VS 2022 C++ x64/x86 Spectre-mitigated libs (v14.29-16.11)`)
4. You can then build and run the `CppSocketLibraryTest` project and it will rebuild the library and run the appropriate tests.

## Usage Examples

- TCP Example using IPV6:

```cpp
void tcpExample()
{
    // Create a new Wifi ServerSocket
    kt::ServerSocket server(kt::SocketType::Wifi, 56756, 20, kt::InternetProtocolVersion::IPV6);

    // Create new TCP socket
    kt::TCPSocket client("::1", server.getPort());

    // Accept the incoming connection at the server
    kt::TCPSocket serverSocket = server.acceptTCPConnection();

    // Send string with text before and after the delimiter
    const std::string testString = "TCP Delimiter Test";
    const char delimiter = '~';
    if (!client.send(testString + delimiter + "other string").first)
    {
        std::cout << "Failed to send test string" << std::endl;
        return;
    }

    if (serverSocket.ready())
    {
        std::string response = serverSocket.receiveToDelimiter(delimiter);
        // Check that the received string is the same as the string sent by the client
        ASSERT_EQ(response, testString);
    }

    // Close all sockets
    client.close();
    serverSocket.close();
    server.close();
}
```

- UDP Example using IPV4 (the default protocol version - so protocol related arguments are omitted):

```cpp
void udpExample() 
{
    // The socket receiving data must first be bound
    kt::UDPSocket socket;
    socket.bind(37893, kt::InternetProtocolVersion::IPV4);

    kt::UDPSocket client;
    const std::string testString = "UDP test string";
    if (!client.sendTo("localhost", 37893, testString).first.first)
    {
        std::cout << "Failed to send to address." << std::endl;
        return;
    }

    if (socket.ready())
    {
        std::pair<std::optional<std::string>, std::pair<int, kt::SocketAddress>> recieved = socket.receiveFrom(testString.size());
        ASSERT_EQ(testString, recieved.first.value());
    }

    socket.close();
}
```

## SIGPIPE Errors

`SIGPIPE` is a signal error raised by UNIX when you attempt to write data to a closed linux socket (closed by the remote). There are a few ways to work around this signal. **Note:** that in both cases, the `TCPSocket.send()` function will return `false` in the result pair so you can detect that the send has failed. *(You can refer to the TCPSocketTest.cpp file and the "...Linux..." related tests to do with "SIGPIPE" to find examples of the below).*

1. Ignore SIGPIPE signals completely:
```cpp
#include <csignal>

...
std::signal(SIGPIPE, SIG_IGN);
...

```

2. Provide `MSG_NOSIGNAL` as the `flag` argument to the `kt::TCPSocket.send()` function which will ensure no signals are raised during the `send()` call.
```cpp
#include <csignal>

...
kt::TCPSocket socket; // Initialise properly
...
if (!socket.send(received, MSG_NOSIGNAL).first)
{
    // Remote has closed connection, you can chose to close it here too
}
...
```

## NOTE: UDP Read Sizes

- Take care when reading UDP messages. If you do not read the entire length of the message the rest of the data will be lost.

## Running the tests Dockerfile

The dockerfile is aimed at testing the socket behaviour across a few different flavours of linux to check it is functioning correctly. Especially since development is being done on WSL and there could be some slight differences.

`docker build -f Environment-Test-Dockerfile .`
