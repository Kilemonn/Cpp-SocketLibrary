# Cpp-SocketLibrary

A ServerSocket and Socket library used to support both Wifi and Bluetooth communication. Currently is only supporting Linux and Windows.

### Set up:

#### Linux
- Run make

#### Windows - WARNING: Windows support is very limitted and still a work in progress.
- Manual compilation
- Required libraries:
	- windows.h
	- winsock2.h
	- ws2tcpip.h
	- iphlpapi.h

### Usage:
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
	// Which ever socket is acting as the "server" needs to bind, only a single socket can be bound 
	// to a specific port at a time
	serverSocket.bind();

	kt::Socket client("127.0.0.1", 43567, kt::SocketType::Wifi, kt::SocketProtocol::UDP);

	const std::string testString = "UDP Test";
    const char delimiter = '~';
    client.send(testString + delimiter);
    const std::string response = server.receiveToDelimiter(delimiter);
    assert(response == testString);

	serverSocket.unbind();
	serverSocket.close();
	client.close();
```

### Upcoming Features:
- Adding to namespace /
- UDP protocol support /
- Improve Windows support
- Improve Bluetooth support

### Known Issues
#### SIGPIPE Error
- There is a known issue regarding SIGPIPE crashing the programming when send() is called after the remote process has already closed the socket.
- A way to resolve this is to remove the SIGPIPE handler, since we can detect this disconnection when the send() function returns false.
- Disable the SIGPIPE handler using the following code:
```cpp
#include <signal.h>

...
signal(SIGPIPE, SIG_IGN);
...

```

#### UDP Read Sizes
- Take care when reading UDP messages. If you do not read the entire length of the message the rest of the data will be lost. Try using receiveAll()/recieveToDelimiter()/receiveAmount() instead of get(), unless you know the amount of data that you are expecting.

## API Documentation

### Socket - API

#### Socket::Socket()
- Default constructor. Should be provided by the compiler, but there has been some scenarios where this is required. A default constructed socket is not useful.


#### Socket::Socket(const std::string&, const unsigned int&, const kt::SocketType, const kt::SocketProtocol = kt::SocketProtocol::None)
- A constructor which will immediately attempt to connect to the host via the port specified.
- **std::string** - The hostname of the device to connect to.
- **unsigned int** - The port number.
- **kt::SocketType** - Determines whether this socket is a wifi or bluetooth socket. 
- **kt::SocketProtocol** - Indicates the protocol being used by this socket, for Wifi this value can be *kt::SocketProtocol::TCP* or *kt::SocketProtocol::UDP* Default value is *kt::SocketProtocol::None*.
- **Throws** SocketException - If the Socket is unable to be instanciated or connect to server.
- **Throws** BindingException - If the Socket is unable to bind to the port specified.


#### Socket::Socket(const int&, const kt::SocketType, const kt::SocketProtocol, const std::string&, const unsigned int&)
- A constructor used by ServerSocket to create and copy of a currently connected socket. **This should not be used directly**.
- **int** - Is the file descriptor for the connection.
- **kt::SocketType** - Determines whether this socket is a wifi or bluetooth socket.
- **kt::SocketProtocol** - Indicates the protocol being used by this socket, for Wifi this value can be *kt::SocketProtocol::TCP* or *kt::SocketProtocol::UDP* Default value is *kt::SocketProtocol::None*.


#### Socket::Socket(const kt::Socket&)
- A copy constructor for the Socket class. Will copy the object members and assume that it is already connected to the endpoint.
- **lt::Socket** - A Socket object to be copied.


#### Socket& Socket::operator=(const Socket&)
- An assignment operator for the Socket object. Will make a copy of the appropriate socket.
- **Socket** - The Socket object to be copied.


#### bool Socket::bind()
- This method is required for *kt::SocketProtocol::UDP* sockets. The socket that is listening for new connections will need to call this before they begin listening (accepting connections). This ensures the socket is bound to the port and can receive new connections. *Only a single process can be bound to a single port at one time*.
- **Throws** *BindingException* if this socket fails to bind.
- **returns** *true* if the socket was bound successfully, otherwise *false*/throws exception


##### bool Socket::unbind()
- This method is required for *kt::SocketProtocol::UDP* sockets. If the socket is bound then the socket will be closed and recreated, this will ensure that the socket is unbound.
**Throws** *SocketException* if the socket is able to be recreated.
**returns** whether the socket is bound or not after this method has been called.


#### void Socket::close()
- Closes the existing connection. If no connection is open, then it will do nothing.
- **This should be closed before the kt::Socket is distructed**


#### bool Socket::ready(unsigned long = 1000) const
- Determines whether the stream has data to read.
- **unsigned long** - The timeout duration in *micro seconds*. Default is 1000 microseconds.
- **Returns** - *true* if there is data to read otherwise *false*.


#### bool Socket::connected(unsigned long = 1000) const
- Determines whether the stream is open.
- **unsigned long** - The timeout duration in *micro seconds*. Default is 1000 microseconds.
- **Returns** - *true* if the stream is open otherwise *false*.
- **NOTE:** This method is still in BETA, and cannot detect if the connection has been closed by the remote device.


#### bool Socket::send(const std::string, int flag = 0) const
- Sends data as a std::string to the receiver.
- **std::string** - The message to send to the receiver.
- **int** - A flag value to specify additional behaviour for this message. *Defaults to 0 if no argument is passed*.
- **Returns** - *true* if the message was sent without error, else *false*.


#### char Socket::get() const
- Reads and returns a single character from the reciever.
- **Returns** - The character read.


#### unsigned int Socket::getPort() const
- **returns** the port number used by this socket.


#### bool Socket::isBound() const
- **returns** *true* if this socket is bound, otherwise *false*. Usage on a *kt::SocketProtocol::TCP* socket will always return *false*.


#### kt::SocketProtocol Socket::getProtocol() const
- **returns** the *kt::SocketProtocol* for this *kt::Socket*.


#### kt::SocketType Socket::getType() const
- **returns** the *kt::SocketType* for this *kt::Socket*.


#### std::string Socket::getLastRecievedAddress() const
- **returns** using *kt::SocketProtocol::UDP* the address of the last device who sent the data that was most recently read. Always returns an empty string for *kt::SocketProtocol::TCP* *kt::Socket*s.


#### std::string Socket::getAddress() const
- **returns** the hostname configured for this socket.


#### std::string Socket::receiveAmount(const unsigned int) const
- Reads in a specific amount of character from the input stream and returns them as a std::string.
- **unsigned int** - The amount of characters to read from the sender.
- **Returns** - A std::string of the specified size with the respective character read in. This method will exit early if there is no more data to send or the other party closes the connection.


#### std::string receiveToDelimiter(const char) const
- Reads from the sender until the passed in delimiter is reached. The delimiter is discarded and the characters preceeding it are returned as a std::string.
- **char** - The delimiter that will be used to mark the end of the read in process.
- **Returns** - A std::string with all of the characters preceeding the delimiter.


#### std::string receiveAll() const
- Reads data while the stream is *ready()*.
- **Returns** - A std::string containing the characters read while the stream was *ready()*.
- **NOTE:** This method can take a long time to execute. If you know the desired size and/or a delimiter, the other methods may be more fitting. This method may take some time for the receiver to retreive the whole message due to the lack of stream flushing through sockets.


#### static std::vector &lt;std::pair&lt;std::string, std::string >> scanDevices(unsigned int duration = 5)
- **In progress**
- Scans for bluetooth devices and returns a std::vector&lt;std::pair&lt;std::string, std::string>> of the device names and addresses.
- **unsigned int** - The duration for which the scan should take to discover nearby bluetooth devices.
- **Returns** - A std::vector&lt;std::pair&lt;std::string, std::string>> where .first is the devices address, and .second is the device name.


### ServerSocket - API

#### ServerSocket::ServerSocket()
- Default constructor. Should be provided by the compiler, but there has been some scenarios where this is required. A default constructed socket is not useful.


#### ServerSocket::ServerSocket(const kt::SocketType, const unsigned int& = 0, const unsigned int& = 20)
- ServerSocket constructor. Creates a wifi/bluetooth ServerSocket and begins listening for connections.
- **kt::SocketType** - Determines whether this ServerSocket is a wifi or bluetooth ServerSocket.
- **unsigned int** - The port number for this server to communicate through. If value is not passed in a random, available port number will be assigned.
- **unsigned int** - You can enter a value here to specify the length of the server connection pool. The default value is 20.
- **Throws** SocketException - If the ServerSocket is unable to be instanciated or begin listening.
- **Throws** BindingException - If the ServerSocket is unable to bind to the specific port specified.


#### ServerSocket::ServerSocket(const ServerSocket&)
- ServerSocket copy constructor.
- **ServerSocket** - The ServerSocket object to be copied.


#### ServerSocket& ServerSocket::operator=(const ServerSocket&)
- Overloaded assignment operator for the ServerSocket class.
- **ServerSocket** - The ServerSocket object to be copied.


#### kt::SocketType ServerSocket::getType() const
- **returns** the *kt::SocketType* for this *kt::Socket*.


#### lt::Socket ServerSocket::accept()
- Used to accept a connection on the specific port. Upon accepting a new connection it will return a Socket object used to communicate with the receiver.
- **Returns** - Socket object of the receiver who has just connected to the ServerSocket.


#### unsigned int ServerSocket::getPort() const
- Used to get the port number that the ServerSocket is listening on.
- **Returns** - An unsigned int of the port number that the ServerSocket is listening on.


#### void ServerSocket::close()
- Closes the existing connection. If no connection is open, then it will do nothing.
- *This should be called before the server goes out of scope*
