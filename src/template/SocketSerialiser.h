//
// Created by Kyle on 3/03/2021.
//

#ifndef CPP_SOCKETLIBRARY_SOCKETSERIALISER_H
#define CPP_SOCKETLIBRARY_SOCKETSERIALISER_H

#include <vector>

namespace kt
{
    template<typename T>
    class SocketSerialiser
    {
    public:
        /**
         * Return an empty vector on error so that the socket will not send this object.
         *
         * @return the byte vector of the passed in object
         */
        virtual std::vector<char> serialise(T) = 0;

        /**
         * Deserialise an object of type T.
         *
         * @return the deserialised object of type T
         */
        virtual T deserialise(std::vector<char>) = 0;
    };
}

#endif //CPP_SOCKETLIBRARY_SOCKETSERIALISER_H
