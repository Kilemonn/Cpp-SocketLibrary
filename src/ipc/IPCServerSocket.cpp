#include "IPCServerSocket.h"
#include "../socketexceptions/SocketError.h"
#include "../socketexceptions/SocketException.hpp"
#include "../socketexceptions/BindingException.hpp"
#include "../socketexceptions/TimeoutException.hpp"

namespace kt
{
    IPCServerSocket::IPCServerSocket(const std::string &socketPath, const bool& override, const unsigned int& connectionBacklogSize, const std::optional<std::function<void(SOCKET &)>> &preBindSocketOperation) : socketPath(socketPath)
    {
        constructSocket(override, connectionBacklogSize, preBindSocketOperation);
    }

    IPCServerSocket::IPCServerSocket(const IPCServerSocket &socket)
    {
        this->socket = socket.socket;
        this->socketPath = socket.socketPath;
    }

    IPCServerSocket &IPCServerSocket::operator=(const IPCServerSocket &socket)
    {
        this->socket = socket.socket;
        this->socketPath = socket.socketPath;

        return *this;
    }

    SOCKET IPCServerSocket::getSocket() const
    {
        return socket;
    }

    std::string IPCServerSocket::getSocketPath() const
    {
        return socketPath;
    }

    void IPCServerSocket::constructSocket(const bool& override, const unsigned int& connectionBacklogSize, const std::optional<std::function<void(SOCKET &)>> &preBindSocketOperation)
    {
        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socketPath.c_str(), std::size(addr.sun_path));

        // When override is true, attempt to remove any existing socket path file before attempting to bind/create it
        if (override)
        {
            IPCSocket::closePath(socketPath.c_str());
        }

        socket = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (isInvalidSocket(this->socket))
        {
            throw kt::SocketException("Error creating IPC server socket: " + getErrorCode());
        }

        if (preBindSocketOperation.has_value())
        {
            preBindSocketOperation.value()(this->socket);
        }

        socklen_t socketSize = sizeof(addr);
        if (bind(this->socket, (sockaddr*)&addr, socketSize) == -1)
        {
            this->close();
            throw kt::BindingException("Error binding to socket path [" + socketPath + "]. " + getErrorCode());
        }

        if (listen(this->socket, connectionBacklogSize) == -1)
        {
            this->close();
            throw kt::SocketException("Error listening on socket path " + socketPath + ": " + getErrorCode());
        }
    }

    IPCSocket IPCServerSocket::accept(const long &timeout) const
    {
        if (timeout > 0)
        {
            int res = Socket::pollSocket(this->socket, timeout);
            if (res == -1)
            {
                throw kt::SocketException("Failed to poll as socket is no longer valid.");
            }
            else if (res == 0)
            {
                throw kt::TimeoutException("No applicable connections could be accepted during the time period specified " + std::to_string(timeout) + " microseconds.");
            }
        }

        sockaddr_un acceptedAddress{};
        socklen_t sockLen = sizeof(acceptedAddress);
        SOCKET temp = ::accept(this->socket, (sockaddr*)&acceptedAddress, &sockLen);
        if (isInvalidSocket(temp))
        {
            throw kt::SocketException("Failed to accept connection. Socket is in an invalid state.");
        }

        return kt::IPCSocket(temp, std::string(acceptedAddress.sun_path));
    }

    void IPCServerSocket::close()
    {
        Socket::close(socket);
        socket = kt::getInvalidSocketValue();
    }
}
