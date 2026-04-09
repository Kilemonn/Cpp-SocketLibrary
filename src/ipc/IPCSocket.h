#pragma once

#include <string>

namespace kt
{
    class IPCSocket
    {
        public:
            int removeSocketPath(const std::string&);
    };
}