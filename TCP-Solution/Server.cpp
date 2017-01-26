//Server

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include <string>

SOCKET Connections[100];
int TotalConnections = 0;

enum Packet
{
	P_ChatMessage,
	P_Test
};

bool SendInt(int ID, int _int)
{
	int RetnCheck = send(Connections[ID], (char*)&_int, sizeof(int), NULL); //send int: _int
	if (RetnCheck == SOCKET_ERROR) //If int failed to send due to connection issue
		return false; //Return false: Connection issue
	return true; //Return true: int successfully sent
}


bool GetInt(int ID, int &_int)
{
	int RetnCheck = recv(Connections[ID], (char*)&_int, sizeof(int), NULL); //receive integer
	if (RetnCheck == SOCKET_ERROR) //If int failed to receive due to connection issue
		return false; //Return false: Connection issue
	return true; //Return true: int successfully received
}

bool SendPacketType(int ID, Packet _packettype)
{
	int RetnCheck = send(Connections[ID], (char*)&_packettype, sizeof(Packet), NULL); //send packet
	if (RetnCheck == SOCKET_ERROR) //If packet failed to send due to connection issue
		return false; //Return false: Connection issue
	return true; //Return true: packet successfully sent
}

bool GetPacketType(int ID, Packet &_packettype)
{
	int RetnCheck = recv(Connections[ID], (char*)&_packettype, sizeof(Packet), NULL); //send packet
	if (RetnCheck == SOCKET_ERROR) //If int failed to receive due to connection issue
		return false; //Return false: Connection issue
	return true; //Return true: packet successfully received
}

bool SendString(int ID, std::string _string)
{
	if (!SendPacketType(ID, P_ChatMessage)) //Send packet type: Chat Message, If sending packet failed
		return false; //return false: failed to send string
	int bufferlength = _string.size(); //Find string buffer length
	if (!SendInt(ID, bufferlength)) //Send length of string buffer, If sending buffer lenght fail then
		return false; //return false: failed to send
	int RetnCheck = send(Connections[ID], _string.c_str(), bufferlength, NULL); //send packet
	if (RetnCheck == SOCKET_ERROR) //If failed to send string buffer
		return false; //Return false: Failed to send string buffer
	return true; //Return true: string successfully sent
}

bool GetString(int ID, std::string & _string)
{
	int bufferlength; //Holds length of the message
	if (!GetInt(ID, bufferlength)) //Get length of buffer and store it in variable: bufferlength
		return false; //If get int fails return false
	char * buffer = new char[bufferlength + 1]; //Allocate buffer
	buffer[bufferlength] = '\0'; //Set last character of buffer to be a null terminator
	int RetnCheck = recv(Connections[ID], buffer, bufferlength, NULL); //receive message and store
	_string = buffer; //set string to received buffer message
	delete[] buffer; //Deallocate buffer memory (cleanup to prevent memory leak)
	if (RetnCheck == SOCKET_ERROR) //If connection is lost while getting message
		return false; //Return false
	return true; //Return true if we were successful in retrieving the string
}


bool ProcessPacket(int ID, Packet packettype)
{
	switch (packettype)
	{
	case P_ChatMessage:
	{
		std::string Message; //string to store our message we received
		if (!GetString(ID, Message)) //Get the chat message and store it in variable Message
			return false; //If we do not properly get the chat message, return false

		//Next we need to send the message out to each user
		for (int i = 0; i < TotalConnections; i++)
		{
			if (i == ID) //If connection is the user who sent the message...
				continue; //Skip to the next user since there is no purpose in sending
			if (!SendString(i, Message)) //Send message to connection at index i,
			{
				std::cout << "Failed to send message from client ID: " << ID << "to client ID: " << i << std::endl;
			}
		}
		std::cout << "Processed chat message packet from user ID: " << ID << std::endl;
		break;
	}
	default:
		std::cout << "Unknown packet: " << packettype << std::endl;
		break;
	}
	return true;
}

void ClientHandlerThread(int ID) //ID = the index in the SOCKET Connections array
{
	Packet packettype;
	while (true)
	{
		if (!GetPacketType(ID, packettype)) //Get packet type
			break; //If there is an issue getting the packet type, exit this loop
		//Once we have the packet type, process the packet
		if (!ProcessPacket(ID, packettype)) //If the packet is not properly processed
			break; //break out for our client handler loop
	}
	std::cout << "Lost connection to client ID: " << ID << std::endl;
	closesocket(Connections[ID]); //close the socket that was being used for the client's connection
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
			Connections[i] = newConnection;
			TotalConnections += 1;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandlerThread, (LPVOID)(i), NULL, NULL); //Create Thread to handle this client. The index in the socket array for this thread is the value (i).
			std::string MOTD = "MOTD: Welcome! This is the message of the day!";
			SendString(i, MOTD);
		}
	}
	system("pause");
	return 0;
}