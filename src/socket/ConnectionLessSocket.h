#pragma once

#include "Socket.h"
#include "../address/SocketAddress.h"

#include <utility>
#include <string>
#include <optional>
#include <functional>

namespace kt
{
    template <typename T>
    class ConnectionLessSocket : public Socket
    {
        public:
            virtual std::pair<int, T> bind(const std::optional<T>& = std::nullopt, const std::optional<std::function<void(SOCKET&)>>& = std::nullopt) = 0;
            virtual bool isBound() const = 0;

            virtual bool ready(const unsigned long = 100) const = 0;

            virtual int sendTo(const T&, const std::string&, const int& = 0) = 0;
            virtual int sendTo(const T&, const char*, const int&, const int& = 0) = 0;
            
            virtual std::pair<std::optional<std::string>, std::pair<int, T>> receiveFrom(const int&, const int& = 0) = 0;
            virtual std::pair<int, T> receiveFrom(char*, const int&, const int& = 0) const = 0;
    };
}
