//Client

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h> //For win sockets
#include <string> //For std::string
#include <iostream> //For std::cout, std::endl, std::cin.getline

SOCKET Connection; //This is client's connection to the server

enum Packet
{
	P_ChatMessage,
	P_Test
};

bool SendInt(int _int)
{
	int RetnCheck = send(Connection, (char*)&_int, sizeof(int), NULL); //send int: _int
	if (RetnCheck == SOCKET_ERROR) //If int failed to send due to connection issue
		return false; //Return false: Connection issue
	return true; //Return true: int successfully sent
}

bool GetInt(int &_int)
{
	int RetnCheck = recv(Connection, (char*)&_int, sizeof(int), NULL); //receive integer
	if (RetnCheck == SOCKET_ERROR) //If int failed to receive due to connection issue
		return false; //Return false: Connection issue
	return true; //Return true: int successfully received
}

bool SendPacketType(Packet _packettype)
{
	int RetnCheck = send(Connection, (char*)&_packettype, sizeof(Packet), NULL); //send packet
	if (RetnCheck == SOCKET_ERROR) //If packet failed to send due to connection issue
		return false; //Return false: Connection issue
	return true; //Return true: packet successfully sent
}

bool GetPacketType(Packet &_packettype)
{
	int RetnCheck = recv(Connection, (char*)&_packettype, sizeof(Packet), NULL); //send packet
	if (RetnCheck == SOCKET_ERROR) //If int failed to receive due to connection issue
		return false; //Return false: Connection issue
	return true; //Return true: packet successfully received
}

bool SendString(std::string _string)
{
	if (!SendPacketType(P_ChatMessage)) //Send packet type: Chat Message, If sending packet failed
		return false; //return false: failed to send string
	int bufferlength = _string.size(); //Find string buffer length
	if (!SendInt(bufferlength)) //Send length of string buffer, If sending buffer lenght fail then
		return false; //return false: failed to send
	int RetnCheck = send(Connection, _string.c_str(), bufferlength, NULL); //send packet
	if (RetnCheck == SOCKET_ERROR) //If failed to send string buffer
		return false; //Return false: Failed to send string buffer
	return true; //Return true: string successfully sent
}

bool GetString(std::string & _string)
{
	int bufferlength; //Holds length of the message
	if (!GetInt(bufferlength)) //Get length of buffer and store it in variable: bufferlength
		return false; //If get int fails return false
	char * buffer = new char[bufferlength + 1]; //Allocate buffer
	buffer[bufferlength] = '\0'; //Set last character of buffer to be a null terminator
	int RetnCheck = recv(Connection, buffer, bufferlength, NULL); //receive message and store
	_string = buffer; //set string to received buffer message
	delete[] buffer; //Deallocate buffer memory (cleanup to prevent memory leak)
	if (RetnCheck == SOCKET_ERROR) //If connection is lost while getting message
		return false; //Return false
	return true; //Return true if we were successful in retrieving the string
}


bool ProcessPacket(Packet packettype)
{
	switch (packettype)
	{
	case P_ChatMessage:
	{
		std::string Message; //string to store our message we received
		if (!GetString(Message)) //get the chat message and store it in variable: Message
			return false; //If we do not properly get the chat message, return false
		std::cout << Message << std::endl; //Display the message to the user
		break;

	}
	default:
		std::cout << "Unknown packet: " << packettype << std::endl;
		break;
	}
	return true;
}


void ClientThread()
{
	Packet packettype;
	while (true)
	{
		//First get the packet type
		if (!GetPacketType(packettype)) // Get packet type
			break;
		if (!ProcessPacket(packettype)) //If the packet is not properly processed
			break;	//break out of out client handler loop
	}
	std::cout << "Lost connection to the server." << std::endl;
	closesocket(Connection); //close the socket that was being used for the client's connection
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

	std::string userinput;
	while (true)
	{
		std::getline(std::cin, userinput); //Get line if user presses enter and fill the buffer
		if (!SendString(userinput)) //Send string: userinput, If string fails to send...
			break;
		Sleep(10);
	}

	system("pause");
	return 0;
}