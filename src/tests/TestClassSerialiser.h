//
// Created by Kyle on 4/03/2021.
//

#ifndef CPP_SOCKETLIBRARY_TESTCLASSSERIALISER_H
#define CPP_SOCKETLIBRARY_TESTCLASSSERIALISER_H

#include "./TestClass.h"
#include "../template/SocketSerialiser.h"

#include <string>
#include <vector>

namespace kt
{
    class TestClassSerialiser : public SocketSerialiser<kt::TestClass>
    {
    public:
        std::vector<char> serialise(kt::TestClass) override;
        kt::TestClass deserialise(std::vector<char>) override;
    };
}

#endif //CPP_SOCKETLIBRARY_TESTCLASSSERIALISER_H
