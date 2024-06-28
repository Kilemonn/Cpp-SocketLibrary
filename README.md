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
    if (!socket.send(testString + delimiter + "other string"))
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
    socket.bind(87893, kt::InternetProtocolVersion::IPV4);

    kt::UDPSocket client;
    const std::string testString = "UDP test string";
    if (!client.sendTo("localhost", 87893, testString).first)
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

## Known Issues

### SIGPIPE Error

- There is a known issue regarding SIGPIPE crashing the programming when send() is called after the remote process has already closed the socket.
- A way to resolve this is to remove the SIGPIPE handler, since we can detect this disconnection when the send() function returns false.

- Disable the SIGPIPE handler using the following code:

```cpp
#include <signal.h>

...
signal(SIGPIPE, SIG_IGN);
...

```

### NOTE: UDP Read Sizes

- Take care when reading UDP messages. If you do not read the entire length of the message the rest of the data will be lost.
