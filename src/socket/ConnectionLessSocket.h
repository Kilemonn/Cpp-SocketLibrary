#include "Socket.h"
#include "../address/SocketAddress.h"

#include <utility>
#include <string>
#include <optional>
#include <functional>

namespace kt
{
    class ConnectionLessSocket : public Socket
    {
        public:
            virtual std::pair<int, kt::SocketAddress> bind(const std::optional<std::string>& = std::nullopt, const unsigned short& = 0, const kt::InternetProtocolVersion = kt::InternetProtocolVersion::Any, const std::optional<std::function<void(SOCKET&)>>& = std::nullopt) = 0;
            virtual bool isUdpBound() const = 0;

            virtual bool ready(const unsigned long = 100) const = 0;

            virtual int sendTo(const std::string&, const kt::SocketAddress&, const int& = 0) = 0;
            virtual int sendTo(const char*, const int&, const kt::SocketAddress&, const int& = 0) = 0;
            virtual std::pair<int, kt::SocketAddress> sendTo(const std::string&, const unsigned short&, const std::string&, const int& = 0, const kt::InternetProtocolVersion = kt::InternetProtocolVersion::Any) = 0;
            virtual std::pair<int, kt::SocketAddress> sendTo(const std::string&, const unsigned short&, const char*, const int&, const int& = 0, const kt::InternetProtocolVersion = kt::InternetProtocolVersion::Any) = 0;
            
            virtual std::pair<std::optional<std::string>, std::pair<int, kt::SocketAddress>> receiveFrom(const int&, const int& = 0) = 0;
            virtual std::pair<int, kt::SocketAddress> receiveFrom(char*, const int&, const int& = 0) const = 0;
    };
}
