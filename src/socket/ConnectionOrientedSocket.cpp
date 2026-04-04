#include "ConnectionOrientedSocket.h"

#include "../socketexceptions/SocketException.hpp"

namespace kt
{
    bool ConnectionOrientedSocket::ready(const unsigned long timeout) const
	{
		int result = this->pollSocket(getSocket(), timeout);
		// 0 indicates that there is no data
		return result > 0;
	}

	bool ConnectionOrientedSocket::connected(const unsigned long timeout) const
	{
		int result = this->pollSocket(getSocket(), timeout);
		// -1 indicates that the connection is not available
		return result != -1;
	}

    int ConnectionOrientedSocket::send(const char* message, const int& messageLength, const int& flags) const
	{
		return ::send(getSocket(), message, messageLength, flags);
	}

	int ConnectionOrientedSocket::send(const std::string& message, const int& flags) const
	{
		return this->send(message.c_str(), message.size(), flags);
	}

    std::optional<char> ConnectionOrientedSocket::get(const int &flags) const
    {
        std::string received = this->receiveAmount(1, flags);
		if (received.empty())
		{
			return std::nullopt;
		}
		return received[0];
    }

    /**
	 * Reads in a specific amount of character from the input stream and returns them as a std::string.
	 * This method will return early if there is no more data to send or the other party closes the connection.
	 *
	 * @param amountToReceive - The amount of characters to read from the sender.
	 *
	 * @return A std::string of the specified size with the respective character read in.
	 */
	std::string kt::ConnectionOrientedSocket::receiveAmount(const unsigned int amountToReceive, const int& flags) const
	{
		std::string data;
		data.resize(amountToReceive);

		int amountReceived = this->receiveAmount(&data[0], amountToReceive, flags);
		return data.substr(0, amountReceived);
    }

    /**
	 * Reads from the sender until the passed in delimiter is reached. The delimiter is discarded and the characters preceeding it are returned as a std::string.
	 *
	 * @param delimiter The delimiter that will be used to mark the end of the read in process.
	 *
	 * @return A std::string with all of the characters preceeding the delimiter.
	 *
	 * @throw SocketException - if the delimiter is '\0'.
	 */
	std::string ConnectionOrientedSocket::receiveToDelimiter(const char& delimiter, const int& flags)
	{
		if (delimiter == '\0')
		{
			throw kt::SocketException("The null terminator '\\0' is an invalid delimiter.");
		}

		std::string data;
		if (!this->ready())
		{
			return data;
		}

		std::optional<char> character;
		do
		{
			character = this->get(flags);
			if (character.has_value() && *character != delimiter)
			{
				data += *character;
			}
		} while (character.has_value() && *character != delimiter && this->ready());

		return data;
	}

    int ConnectionOrientedSocket::receiveAmount(char* buffer, const unsigned int amountToReceive, const int& flags) const
	{
		int counter = 0;
		if (amountToReceive == 0)
		{
			return counter;
		}
		
		do
		{
			int amountReceived = ::recv(getSocket(), &buffer[counter], static_cast<int>(amountToReceive - counter), flags);
			if (amountReceived < 1)
			{
				return counter;
			}
			counter += amountReceived;
		} while (counter < amountToReceive && this->ready());

		return counter;
	}

    /**
	 * Reads data while the stream is *ready()*.
	 *
	 * @return A std::string containing the characters read while the stream was *ready()*.
	 *
	 * **NOTE:** This method can take a long time to execute. If you know the desired size and/or a delimiter, the other methods may be more fitting.
	 * This method may take some time for the receiver to retreive the whole message due to the inability to flush streams when using sockets.
	 */
	std::string kt::ConnectionOrientedSocket::receiveAll(const unsigned long timeout, const int& flags)
	{
		std::string result;
		result.reserve(1024);
		bool hitEOF = false;

		while (this->ready(timeout) && !hitEOF)
		{
			std::string res = receiveAmount(this->pollSocket(getSocket(), timeout), flags);
			if (!res.empty() && res[0] == '\0')
			{
				hitEOF = true;
			}
			else
			{
				result += res;
			}
		}
		result.shrink_to_fit();
		return result;
	}
}
