#pragma once

#include "../serversocket/ServerSocket.h"
#include "IPCSocket.h"

#include <string>
#include <optional>
#include <functional>

namespace kt
{
    class IPCServerSocket : public ServerSocket<IPCSocket>
    {
        private:
            SOCKET socket;
            std::string socketPath;

            void constructSocket(const bool& = false, const unsigned int& = 0, const std::optional<std::function<void(SOCKET&)>>& = std::nullopt);
            
        public:
            IPCServerSocket() = delete;
            IPCServerSocket(const std::string&, const bool& = false, const unsigned int& = 0, const std::optional<std::function<void(SOCKET&)>>& = std::nullopt);

            IPCServerSocket(const IPCServerSocket&);
			IPCServerSocket& operator=(const IPCServerSocket&);

            SOCKET getSocket() const;
            std::string getSocketPath() const;

            IPCSocket accept(const long& = 0) const override;

            void close() override;
    };
}
