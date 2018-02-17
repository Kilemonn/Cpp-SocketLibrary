
#include <iostream>
#include <thread>

#include "Socket/Socket.h"
#include "ServerSocket/ServerSocket.h"

void serverFunction();

int main()
{
	std::thread t1(serverFunction);

	std::cout << "HERE!!!" << std::endl;

	Socket socket("127.0.0.1", 3000, true);
	std::cout << "Connected" << std::endl;

	std::string received = socket.receiveToDelimiter('\n');
	std::cout << "RECIEVED: " << received << std::endl;

	socket.sendMessage("12345");
	socket.closeSocket();

	t1.join();

	return 0;
}

void serverFunction()
{
	ServerSocket server(3000, true);

	Socket client(server.acceptWifiConnection());
	std::cout << "Accepted" << std::endl;

	client.sendMessage("HEY\n");

	std::string res = client.receiveAmount(4);
	std::cout << "RES: " << res << std::endl;
}
