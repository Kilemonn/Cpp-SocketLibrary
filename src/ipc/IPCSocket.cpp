#include "IPCSocket.h"
#include "../socket/Socket.h"
#include "../socketexceptions/SocketError.h"
#include "../socketexceptions/SocketException.hpp"

#include <cstring>

namespace kt
{
    IPCSocket::IPCSocket(const std::string& socketPath) : socketPath(socketPath) 
    {
        constructSocket();
    }

    IPCSocket::IPCSocket(const SOCKET &socket, const std::string &socketPath) : socket(socket), socketPath(socketPath)
    {

    }

    IPCSocket::IPCSocket(const IPCSocket &socket)
    {
        this->socket = socket.socket;
        this->socketPath = socket.socketPath;
    }

    IPCSocket &IPCSocket::operator=(const IPCSocket &socket)
    {
        this->socket = socket.socket;
        this->socketPath = socket.socketPath;

        return *this;
    }

    SOCKET IPCSocket::getSocket() const
    {
        return socket;
    }

    std::string IPCSocket::getSocketPath() const
    {
        return socketPath;
    }

    void IPCSocket::close()
    {
        Socket::close(socket);
        this->socket = getInvalidSocketValue();
    }

    void IPCSocket::closePath(const std::string &socketPath)
    {
        unlink(socketPath.c_str());
    }

    void IPCSocket::constructSocket()
    {
        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;

        if (socketPath.size() >= std::size(addr.sun_path))
        {
            socketPath.resize(std::size(addr.sun_path));
        }

#ifdef _WIN32
        strcpy_s(addr.sun_path, std::size(addr.sun_path), socketPath.c_str());
#else
        strncpy(addr.sun_path, socketPath.c_str(), std::size(addr.sun_path));
#endif

        socket = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (!isInvalidSocket(this->socket))
        {
            int connectionResult = connect(socket, (sockaddr*)&addr, sizeof(addr));
            if (connectionResult == 0)
            {
                return;
            }
        }

        throw kt::SocketException("Unable to connect to provided IPC path: [" + this->socketPath + "] " + getErrorCode());
    }
} // namespace kt
