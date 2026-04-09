#include "IPCSocket.h"

namespace kt
{
    int IPCSocket::removeSocketPath(const std::string &socketPath)
    {
        return std::remove(socketPath.c_str());
    }
}
