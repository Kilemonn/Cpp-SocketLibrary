//
// Created by Kyle on 4/03/2021.
//

#ifndef CPP_SOCKETLIBRARY_TESTCLASS_H
#define CPP_SOCKETLIBRARY_TESTCLASS_H

#include <string>

namespace kt
{
    /**
     * A class used to test the `SocketSerialiser` using non-set size elements.
     */
    class TestClass
    {
    public:
        int* intArray;
        char* charArray;
        std::string str;

        TestClass(int*, char*, std::string);
    };
}

#endif //CPP_SOCKETLIBRARY_TESTCLASS_H
