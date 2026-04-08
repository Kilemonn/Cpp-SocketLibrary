#include "DatagramIPCSocket.h"

#include "../socketexceptions/BindingException.hpp"

namespace kt
{
    std::pair<int, std::string> DatagramIPCSocket::bind(const std::optional<std::string> &socketPath, const std::optional<std::function<void(SOCKET &)>> &preBindSocketOperation)
    {
        return bind(false, socketPath, preBindSocketOperation);
    }

    std::pair<int, std::string> DatagramIPCSocket::bind(const bool &override, const std::optional<std::string> &socketPathOpt, const std::optional<std::function<void(SOCKET &)>> &preBindSocketOperation)
    {
        if (!socketPathOpt.has_value())
		{
			throw kt::BindingException("Failed to bind to empty socket path");
		}
        std::string path = socketPathOpt.value();

        sockaddr_un address{};
        address.sun_family = AF_UNIX;

        if (path.size() >= std::size(address.sun_path))
        {
            path.resize(std::size(address.sun_path));
        }

#ifdef _WIN32
        strcpy_s(address.sun_path, std::size(address.sun_path), path.c_str());
#else
        strncpy(address.sun_path, path.c_str(), std::size(address.sun_path));
#endif

        // When override is true, attempt to remove any existing socket path file before attempting to bind/create it
        if (override)
        {
            IPCSocket::removeSocketPath(path);
        }

        receiveSocket = ::socket(AF_UNIX, SOCK_DGRAM, 0);
        if (isInvalidSocket(this->receiveSocket))
        {
            throw kt::SocketException("Error creating IPC server socket: " + getErrorCode());
        }

        if (isBound())
        {
            this->close();
        }

        if (preBindSocketOperation.has_value())
        {
            preBindSocketOperation.value()(this->receiveSocket);
        }

        socklen_t socketSize = sizeof(address);
        int bindResult = ::bind(this->receiveSocket, (sockaddr*)&address, socketSize);
		this->bound = bindResult != -1;
		if (!this->bound)
		{
			return std::make_pair(bindResult, "");
		}

        this->socketPath = path;
		return std::make_pair(bindResult, socketPath.value());
    }

    bool DatagramIPCSocket::isBound() const
    {
        return bound;
    }

    SOCKET DatagramIPCSocket::getListeningSocket() const
    {
        return receiveSocket;
    }

    std::optional<std::string> DatagramIPCSocket::getSocketPath() const
    {
        return socketPath;
    }

    void DatagramIPCSocket::setPreSendSocketOperation(std::function<void(SOCKET &)> preSendSocketOperation)
    {
        this->preSendSocketOperation = preSendSocketOperation;
    }

    bool DatagramIPCSocket::ready(const unsigned long timeout) const
    {
        if (!isBound())
		{
			return false;
		}
		
		int result = this->pollSocket(this->receiveSocket, timeout);
		// 0 indicates that there is no data
		return result > 0;
    }

    int DatagramIPCSocket::sendTo(const std::string &path, const std::string &message, const int &flags)
    {
        return sendTo(path, message.c_str(), message.size(), flags);
    }

    int DatagramIPCSocket::sendTo(const std::string &path, const char *buffer, const int &bufferLength, const int &flags)
    {
        SOCKET tempSocket = socket(AF_UNIX, SOCK_DGRAM, 0);
		if (kt::isInvalidSocket(tempSocket))
		{
			return -2;
		}

		if (preSendSocketOperation.has_value())
		{
			preSendSocketOperation.value()(tempSocket);
		}

        sockaddr_un address{};
        address.sun_family = AF_UNIX;

        std::string sendPath = path;
        if (sendPath.size() >= std::size(address.sun_path))
        {
            sendPath.resize(std::size(address.sun_path));
        }

#ifdef _WIN32
        strcpy_s(address.sun_path, std::size(address.sun_path), sendPath.c_str());
#else
        strncpy(address.sun_path, sendPath.c_str(), std::size(address.sun_path));
#endif

		int result = ::sendto(tempSocket, buffer, bufferLength, flags, (sockaddr*)&address, sizeof(address));
		Socket::close(tempSocket);
		return result;
    }

    std::pair<std::optional<std::string>, std::pair<int, std::string>> DatagramIPCSocket::receiveFrom(const int &receiveLength, const int &flags)
    {
        std::string data;
		data.resize(receiveLength);

		std::pair<int, std::string> result = this->receiveFrom(&data[0], receiveLength, flags);

		// Need to substring to remove any null terminating bytes
		if (result.first >= 0 && result.first < receiveLength)
		{
			data = data.substr(0, result.first);
		}

		return std::make_pair(data.size() == 0 ? std::nullopt : std::make_optional(data), result);
    }

    std::pair<int, std::string> DatagramIPCSocket::receiveFrom(char *buffer, const int &receiveLength, const int &flags) const
    {
		if (!isBound() || receiveLength == 0)
		{
			return std::make_pair(-1, "");
		}

        sockaddr_un receiveAddress{};
        // Using auto here since the "addressLength" argument for "::recvfrom()" has differing types depending what platform
		// we are on, so I am letting the definition of kt::getAddressLength() drive this type via auto
		socklen_t addressLength = sizeof(receiveAddress);
        int flag = ::recvfrom(this->receiveSocket, buffer, receiveLength, flags, (sockaddr*)&receiveAddress, &addressLength);
		return std::make_pair(flag, std::string{receiveAddress.sun_path});
    }

    void DatagramIPCSocket::close()
    {
        Socket::close(this->receiveSocket);
		this->receiveSocket = kt::getInvalidSocketValue();

        if (isBound() && socketPath.has_value())
        {
            IPCSocket::removeSocketPath(socketPath.value());
        }
		socketPath = std::nullopt;
		this->bound = false;
    }
}
