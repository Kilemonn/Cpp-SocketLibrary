#include "DatagramIPCSocket.h"

namespace kt
{
    bool DatagramIPCSocket::isBound() const
    {
        return bound;
    }

    void DatagramIPCSocket::setPreSendSocketOperation(std::function<void(SOCKET &)> preSendSocketOperation)
    {
        this->preSendSocketOperation = preSendSocketOperation;
    }

    void DatagramIPCSocket::close()
    {
        Socket::close(this->receiveSocket);
		this->receiveSocket = kt::getInvalidSocketValue();

        if (isBound() && socketPath.has_value())
        {
            IPCSocket::removeSocketPath(socketPath.value());
        }
		
		this->bound = false;
    }
}
