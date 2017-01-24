//Server

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <iostream>

SOCKET Connections[100];
int ConnectionCounter = 0;

void ClientHandlerThread(int index) //index = the index in the SOCKET Connections array
{
	int bufferlength; //Holds the length of the buffer
	while (true)
	{

		recv(Connections[index], (char*)&bufferlength, sizeof(int), NULL); //get buffer length
		char * buffer = new char[bufferlength+1]; //Buffer to hold received message
		buffer[bufferlength] = '\0'; //set last character of buffetr to be null terminator
		recv(Connections[index], buffer, bufferlength, NULL); //Receive message from client

		for (int i = 0; i < ConnectionCounter; i++) //For each client connection
		{
			if (i == index)	//skipping sending to the user who sent the message
				continue;
			send(Connections[i], (char*)&bufferlength, sizeof(int), NULL); //send bufferlength to i-th client (except the sending user)
			send(Connections[i], buffer, bufferlength, NULL); //send message to i-th client (except the sending user)
		}
		delete[] buffer;
	}
}

int main()
{
	//Winsock Startup
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0) //If WSAStartup returns anything other than 0, then that means an error has occured in WINSock Startup function
	{
		MessageBoxA(NULL, "Winsock startup failed", "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	//Socket address information
	SOCKADDR_IN addr; //Address that we will bind our listening socket to
	int addrlen = sizeof(addr); //length of the address (required for accept call)
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //Broadcast locally
	addr.sin_port = htons(1111); //Port
	addr.sin_family = AF_INET; //IPv4 Socket

							   //Create a socket, which will listen
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL); //Create socket to listen for new connections
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr)); //Bind the address to the socket
	listen(sListen, SOMAXCONN); //Places sListen socket in a state in which it is listening for an incoming connection. Note: SOMAXCONN = Socket Outstanding Max Connections

	SOCKET newConnection; //Socket to hold the client's connection
	for (int i = 0; i < 100; i++)
	{
		newConnection = accept(sListen, (SOCKADDR*)&addr, &addrlen); //Accept a new connection
		if (newConnection == 0) //If accepting the client failed
		{
			std::cout << "Failed to accept the client's connection." << std::endl;
		}
		else //If client connection properly accepted
		{
			std::cout << "Client Connected!" << std::endl;
			std::string MOTD = "Welcome! This is the Message of the Day."; //Create buffer with message of the day
			int MOTDLength = MOTD.size();
			send(newConnection, (char*)&MOTDLength, sizeof(int), NULL); //Send MOTD length
			send(newConnection, MOTD.c_str(), MOTDLength, NULL); //Send MOTD buffer
			Connections[i] = newConnection;
			ConnectionCounter += 1;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandlerThread, (LPVOID)(i), NULL, NULL); //Create Thread to handle this client. The index in the socket array for this thread is the value (i).
		}
	}
	system("pause");
	return 0;
}