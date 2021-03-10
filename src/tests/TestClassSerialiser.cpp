//
// Created by Kyle on 4/03/2021.
//

#include <algorithm>
#include <iostream>

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
        // Some scuffed deserialiser
        int ints[10];
        unsigned int intsIndex = 0;
        char chars[20];
        unsigned int charsIndex = 0;
        std::vector<char>::iterator it = bytes.begin();
        std::vector<char>::iterator endOfInts = std::find(bytes.begin(), bytes.end(), '\0');

        std::vector<char>::iterator nextNumberEnds;
        do
        {
            std::string temp = "";
            nextNumberEnds = std::find(it, endOfInts, '-');

            if (nextNumberEnds != endOfInts)
            {
                for_each(it, nextNumberEnds, [&temp](char c)
                {
                   temp.push_back(c);
                });
                ints[intsIndex] = std::stoi(temp, nullptr);
                intsIndex++;
                // Move iterator past the "-"
                nextNumberEnds++;
                it = nextNumberEnds;
            }

        } while (nextNumberEnds != endOfInts);

        // Skip "\0"
        if (endOfInts != bytes.end())
        {
            endOfInts++;
        }

        std::vector<char>::iterator endOfChars = std::find(endOfInts, bytes.end(), '\0');
        for_each(endOfInts, endOfChars, [&chars, &charsIndex](char c)
        {
           chars[charsIndex] = c;
           charsIndex++;
        });

        // Skip "\0"
        if (endOfChars != bytes.end())
        {
            endOfChars++;
        }

        std::string temp = "";
        for_each(endOfChars, bytes.end(), [&temp](char c)
        {
            temp.push_back(c);
        });

        std::cout << ints[0] << std::endl;
        std::cout << chars[0] << std::endl;
        std::cout << temp << std::endl;

        return kt::TestClass(ints, chars, temp);
    }
}
