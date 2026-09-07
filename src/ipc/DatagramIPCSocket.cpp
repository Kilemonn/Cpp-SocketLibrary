#include "DatagramIPCSocket.h"

#include "../socketexceptions/BindingException.hpp"

namespace kt
{
    DatagramIPCSocket::DatagramIPCSocket()
    {
#ifdef _WIN32
        throw SocketException("DatagramIPCSocket is not supported on Windows.");
#endif
    }

    std::expected<std::string, int> DatagramIPCSocket::bind(const std::optional<std::string> &socketPath, const std::optional<std::function<void(SOCKET &)>> &preBindSocketOperation)
    {
        return bind(false, socketPath, preBindSocketOperation);
    }

    std::expected<std::string, int> DatagramIPCSocket::bind(const bool &override, const std::optional<std::string> &socketPathOpt, const std::optional<std::function<void(SOCKET &)>> &preBindSocketOperation)
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
    	
    	// If we are bound then close the current socket before we create a new one
    	if (isBound())
    	{
    		this->close();
    	}

        receiveSocket = ::socket(AF_UNIX, SOCK_DGRAM, 0);
        if (isInvalidSocket(this->receiveSocket))
        {
            return std::unexpected(getErrorCodeValue());
        }

        if (preBindSocketOperation.has_value())
        {
            preBindSocketOperation.value()(this->receiveSocket);
        }

        const socklen_t socketSize = sizeof(address);
        int bindResult = ::bind(this->receiveSocket, reinterpret_cast<sockaddr*>(&address), socketSize);
		this->bound = bindResult != -1;
		if (!this->bound)
		{
			return std::unexpected(getErrorCodeValue());
		}

        this->socketPath = path;
		return socketPath.value();
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

		int result = ::sendto(tempSocket, buffer, bufferLength, flags, reinterpret_cast<sockaddr*>(&address), sizeof(address));
		Socket::close(tempSocket);
		return result;
    }

    std::expected<std::pair<std::string, std::string>, int> DatagramIPCSocket::receiveFrom(const int &receiveLength, const int &flags)
    {
        std::string data;
		data.resize(receiveLength);

		std::pair<std::string, int> result = this->receiveFrom(data.data(), receiveLength, flags);

		// Need to substring to remove any null trailing bytes
		if (result.second >= 0)
		{
            if (result.second < receiveLength)
            {
                data = data.substr(0, result.second);
            }
		}
        else
        {
            return std::unexpected(kt::getErrorCodeValue());
        }

		return std::make_pair(data, result.first);
    }

    std::pair<std::string, int> DatagramIPCSocket::receiveFrom(char *buffer, const int &receiveLength, const int &flags) const
    {
		if (!isBound() || receiveLength == 0)
		{
			return std::make_pair("", -1);
		}

        sockaddr_un receiveAddress{};
        // Using auto here since the "addressLength" argument for "::recvfrom()" has differing types depending what platform
		// we are on, so I am letting the definition of kt::getAddressLength() drive this type via auto
		socklen_t addressLength = sizeof(receiveAddress);
        int flag = ::recvfrom(this->receiveSocket, buffer, receiveLength, flags, reinterpret_cast<sockaddr*>(&receiveAddress), &addressLength);

        // Just return "socketPath" since we know that messages can only come from that path since we are bound to it
		return std::make_pair(socketPath.value(), flag);
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
