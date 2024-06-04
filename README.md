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

1. To build the library, firstly run cmake: `cmake . -B build` in the root directory of the repository (`CppSocketLibrary/`).
2. Then move into the new `build` folder: `cd build`.
3. Then you can run `make` to build the library.
4. Then you can run `make check`.

### Building the Library and Running the Tests - Windows

1. To build the library, firstly run cmake: `cmake . -B build` in the root directory of the repository (`CppSocketLibrary/`).
2. This will create a `.sln` file which can be opened in `Visual Studio`.
3. Once opened (and with the appropriate windows service packs installed - `MSVC v143 - VS 2022 C++ x64/x86 Spectre-mitigated libs (v14.29-16.11)`)
4. You can then build and run the `CppSocketLibraryTest` project and it will rebuild the library and run the appropriate tests.

## Usage Examples

- TCP Example using IPV6:

```cpp
// Create a new Wifi ServerSocket
kt::ServerSocket server(kt::SocketType::Wifi, 56756, 20, InternetProtocolVersion::IPV6);

// Create new TCP socket
kt::Socket client("::1", server.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::TCP, InternetProtocolVersion::IPV6);

// Accept connection to server
kt::Socket serverSocket = server.accept();

const std::string testString = "Test";
serverSocket.send(testString);
const std::string response = client.receiveAmount(testString.size());
// Compare received and sent string values
assert(response == testString);

client.close();
serverSocket.close();
server.close();
```

- UDP Example using IPV4 (the default protocol version - so protocol related arguments are omitted):

```cpp
kt::Socket serverSocket("127.0.0.1", 43567, kt::SocketType::Wifi, kt::SocketProtocol::UDP);
// Which ever socket is acting as the "server" needs to bind, only a single process can be bound 
// to a specific port at a time
serverSocket.bind();

kt::Socket client("127.0.0.1", 43567, kt::SocketType::Wifi, kt::SocketProtocol::UDP);

const std::string testString = "UDP Test";
const char delimiter = '~';
client.send(testString + delimiter);
const std::string response = serverSocket.receiveToDelimiter(delimiter);
assert(response == testString);

serverSocket.close();
client.close();
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

- Take care when reading UDP messages. If you do not read the entire length of the message the rest of the data will be lost. Try using `receiveAll()`/`recieveToDelimiter()`/`receiveAmount()` instead of `get()`, unless you know the amount of data that you are expecting.
