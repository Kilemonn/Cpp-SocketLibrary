//
// Created by Kyle on 4/03/2021.
//

#include "TestClassSerialiser.h"

namespace kt
{
    std::vector<char> TestClassSerialiser::serialise(kt::TestClass tc)
    {
        std::vector<char> bytes;

        // Serialise ints
        for(unsigned int i = 0; i < sizeof(tc.intArray); i++)
        {
            std::string intString = std::to_string(tc.intArray[i]);
            for (char c : intString)
            {
                bytes.push_back(c);
            }
            bytes.push_back('-');
        }
        bytes.push_back('\0');

        // Serialise chars
        for(unsigned int i = 0; i < sizeof(tc.charArray); i++)
        {
            bytes.push_back(tc.charArray[i]);
        }
        bytes.push_back('\0');

        for(char c : tc.str)
        {
            bytes.push_back(c);
        }

        return bytes;
    }

    kt::TestClass TestClassSerialiser::deserialise(std::vector<char> bytes)
    {
        return kt::TestClass(nullptr, nullptr, "");
    }
}
