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

- Create a new Wifi Socket:

```cpp
	Socket socket("127.0.0.1", 52123, Socket::WIFI);
```

- Create a new Wifi ServerSocket:

```cpp
	ServerSocket server(ServerSocket::WIFI);
```

### Upcoming Features:

- Adding to namespace
- UDP protocol support
- Bluetooth support on Windows
- MacOS support


## API Documentation

### Socket - API

#### Socket(const std::string&, const int&, bool)

- A constructor which will immediately attempt to connect to the host via the port specified.
- **std::string** - The hostname of the device to connect to.
- **int** - The port number.
- **bool** - Determines whether this socket is a wifi or bluetooth socket. *true* is Wifi, *false* is Bluetooth. (You can use Socket::Wifi and Socket::Bluetooth to get the expect values).
- **Throws** SocketException - If the Socket is unable to be instanciated or connect to server.
- **Throws** BindingException - If the Socket is unable to bind to the specific port specified.


#### Socket(const SOCKET&/int&, bool)

- A constructor used by ServerSocket to create and copy of a currently connected socket.
- **SOCKET/int** - Is the file descriptor for the connection.
- **bool** - Determines whether this socket is a wifi or bluetooth socket. *true* is Wifi, *false* is Bluetooth. (You can use Socket::Wifi and Socket::Bluetooth to get the expect values).


#### Socket(const Socket&)

- A copy constructor for the Socket class. Will copy the object members and assume that it is already connected to the endpoint.
- **Socket** - A Socket object to be copied.


#### Socket& operator=(const Socket&)

- An assignment operator overload for the Socket object. Will close the current instance of Socket then copy the members.
- **Socket** - A Socket object to be copied.


#### ~Socket()

- Socket object destructor. Ensures that the connection is closed before the resources are freed.


#### void close()

- Closes the existing connection. If no connection is open, then it will do nothing.


#### bool ready(unsigned long = 0) const

- Determines whether the stream has data to read.
- **unsigned long** - The timeout duration in *micro seconds*. Default is zero.
- **Returns** - *true* if there is data to read and *false* otherwise.


#### bool send(const std::string, int flag = 0) const

- Sends data as a std::string to the receiver.
- **std::string** - The message to send to the receiver.
- **int** - A flag value to specify additional behaviour for this message. *Defaults to 0 if no argument is passed*.
- **Returns** - *true* if the message was sent without error, else *false*.


#### char get() const

- Reads and returns a single character from the reciever.
- **Returns** - The character read.


#### std::string receiveAmount(const unsigned int) const

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

- Scans for bluetooth devices and returns a std::vector&lt;std::pair&lt;std::string, std::string>> of the device names and addresses.
- **unsigned int** - The duration for which the scan should take to discover nearby bluetooth devices.
- **Returns** - A std::vector&lt;std::pair&lt;std::string, std::string>> where .first is the devices address, and .second is the device name.


### ServerSocket - API

#### ServerSocket(const bool, const unsigned int& = 0)

- ServerSocket constructor. Creates a wifi/bluetooth ServerSocket and begins listening for connections.
- **bool** - Determines whether this ServerSocket is a wifi or bluetooth ServerSocket. *true* is Wifi, *false* is Bluetooth. (You can use Socket::Wifi and Socket::Bluetooth to get the expect values).
- **unsigned int** - The port number for this server to communicate through. If value is not passed in a random, available port number will be assigned.
- **Throws** SocketException - If the ServerSocket is unable to be instanciated or begin listening.
- **Throws** BindingException - If the ServerSocket is unable to bind to the specific port specified.


#### ServerSocket(const ServerSocket&)

- ServerSocket copy constructor.
- **ServerSocket** - The ServerSocket object to be copied.


#### ServerSocket& operator=(const ServerSocket&)

- Overloaded assignment operator for the ServerSocket class.
- **ServerSocket** - The ServerSocket object to be copied.


#### ~ServerSocket()

- A ServerSocket destructor, ensures the connection is closed before releasing the resources.


#### Socket accept()

- Used to accept a connection on the specific port. Upon accepting a new connection it will return a Socket object used to communicate with the receiver.
- **Returns** - Socket object of the receiver who has just connected to the ServerSocket.


#### unsigned int getPort() const

- Used to get the port number that the ServerSocket is listening on.
- **Returns** - An unsigned int of the port number that the ServerSocket is listening on.


#### void close()

- Closes the existing connection. If no connection is open, then it will do nothing.
