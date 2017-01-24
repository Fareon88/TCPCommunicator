//Client

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h> //For win sockets
#include <string> //For std::string
#include <iostream> //For std::cout, std::endl, std::cin.getline

SOCKET Connection; //This is client's connection to the server

void ClientThread()
{
	int bufferlength; //Holds the length of the buffer
	while (true)
	{
		recv(Connection, (char*)&bufferlength, sizeof(int), NULL); //receive buffer length
		char * buffer = new char[bufferlength+1]; //Allocates buffer
		buffer[bufferlength] = '\0';
		recv(Connection, buffer, bufferlength, NULL); //receive message
		std::cout << buffer << std::endl; //print out buffer
		delete[] buffer; //Deallocate buffer
	}
}

int main()
{
	system("pause");

	//Winsock Startup
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0) //If WSAStartup returns anything other than 0, then that means an error has occured in WINSock Startup function
	{
		MessageBoxA(NULL, "Winsock startup failed", "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	//Soccet address information
	SOCKADDR_IN addr; //Address that we will bind our listening socket to
	int sizeofaddr = sizeof(addr); //Need sizeofaddr for the connect function
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //Address = localhost (this PC)
	addr.sin_port = htons(1111); //Port = 1111
	addr.sin_family = AF_INET; //IPv4 Socket

	//Establish socket for the connection
	Connection = socket(AF_INET, SOCK_STREAM, NULL); //Set Connection socket
	if (connect(Connection, (SOCKADDR*)&addr, sizeofaddr) != 0) //If we are unable to connect...
	{
		MessageBoxA(NULL, "Failed to Connect", "Error", MB_OK | MB_ICONERROR);
		return 0; //Failed to Connect
	}
	std::cout << "Connected!" << std::endl;
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientThread, NULL, NULL, NULL); //Create the client thread

	std::string buffer;
	while (true)
	{
		std::getline(std::cin, buffer); //Get line if user presses enter and fill the buffer
		int bufferlength = buffer.size();
		send(Connection, (char*)&bufferlength, sizeof(int), NULL); //Send integer that holds the size in bytes
		send(Connection, buffer.c_str(), bufferlength, NULL); //Send buffer
		Sleep(10);
	}

	system("pause");
	return 0;
}