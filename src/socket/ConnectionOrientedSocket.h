#pragma once

#include "Socket.h"

#include <optional>
#include <string>

namespace kt
{
    class ConnectionOrientedSocket : public Socket
    {
        public:
            virtual SOCKET getSocket() const = 0;

            virtual bool ready(const unsigned long = 100) const;
			virtual bool connected(const unsigned long = 100) const;

            virtual int send(const char*, const int&, const int& = 0) const;
			virtual int send(const std::string&, const int& = 0) const;

            virtual std::optional<char> get(const int& = 0) const;
			virtual std::string receiveAmount(const unsigned int, const int& = 0) const;
			virtual std::string receiveToDelimiter(const char&, const int& = 0);

            virtual int receiveAmount(char*, const unsigned int, const int& = 0) const;
			virtual std::string receiveAll(const unsigned long = 100, const int& = 0);
    };
}
