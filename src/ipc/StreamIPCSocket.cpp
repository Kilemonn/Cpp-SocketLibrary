#include "StreamIPCSocket.h"
#include "../socket/Socket.h"
#include "../socketexceptions/SocketError.h"
#include "../socketexceptions/SocketException.hpp"

#include <cstring>

namespace kt
{
    StreamIPCSocket::StreamIPCSocket(const std::string& socketPath) : socketPath(socketPath) 
    {
        constructSocket();
    }

    StreamIPCSocket::StreamIPCSocket(const SOCKET &socket, const std::string &socketPath) : socket(socket), socketPath(socketPath)
    {

    }

    StreamIPCSocket::StreamIPCSocket(const StreamIPCSocket &socket)
    {
        this->socket = socket.socket;
        this->socketPath = socket.socketPath;
    }

    StreamIPCSocket &StreamIPCSocket::operator=(const StreamIPCSocket &socket)
    {
        this->socket = socket.socket;
        this->socketPath = socket.socketPath;

        return *this;
    }

    SOCKET StreamIPCSocket::getSocket() const
    {
        return socket;
    }

    std::string StreamIPCSocket::getSocketPath() const
    {
        return socketPath;
    }

    void StreamIPCSocket::close()
    {
        Socket::close(socket);
        this->socket = getInvalidSocketValue();
    }

    void StreamIPCSocket::closePath(const std::string &socketPath)
    {
        unlink(socketPath.c_str());
    }

    void StreamIPCSocket::constructSocket()
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
