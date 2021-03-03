# Cpp-SocketLibrary

A ServerSocket and Socket library for Linux aiming to support both Wifi and Bluetooth communication.

## Getting Started

### Dependencies

The following dependencies are required:
- `libbluetooth-dev`
- `libglib2.0-dev`
- `bluez`

### Building the Library

To build the library run: `cmake . -B build`
Then change to the `build` folder, `cd build`.
Then you can run `make` to build the library.

## Usage Examples

- TCP Example:

```cpp
// Create a new Wifi ServerSocket
kt::ServerSocket server(kt::SocketType::Wifi, 56756);

// Create new TCP socket
kt::Socket client("127.0.0.1", 56756, kt::SocketType::Wifi, kt::SocketProtocol::TCP);

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

- UDP Example:

```cpp
kt::Socket serverSocket("127.0.0.1", 43567, kt::SocketType::Wifi, kt::SocketProtocol::UDP);
// Which ever socket is acting as the "server" needs to bind, only a single process can be bound 
// to a specific port at a time
serverSocket.bindUdpSocket();

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
