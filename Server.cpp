#include "Server.h"



Server::Server(int port, bool broadcastPublicly) {
	this->port = port;

	//Winsock Startup
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0) //If WSAStartup returns anything other than 0, then that means an error has occured in the WinSock Startup.
	{
		MessageBoxA(NULL, "WinSock startup failed", "Error", MB_OK | MB_ICONERROR);
		exit(0);
	}

	if (broadcastPublicly) {
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else {
		addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //Broadcast locally
	}

	addr.sin_port = htons(port); //Port
	addr.sin_family = AF_INET; //IPv4 Socket

	sListen = socket(AF_INET, SOCK_STREAM, NULL); //Create socket to listen for new connections
	if (bind(sListen, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) //Bind the address to the socket, if we fail to bind the address..
	{
		std::string ErrorMsg = "Failed to bind the address to our listening socket. Winsock Error:" + std::to_string(WSAGetLastError());
		MessageBoxA(NULL, ErrorMsg.c_str(), "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}
	if (listen(sListen, SOMAXCONN) == SOCKET_ERROR) //Places sListen socket in a state in which it is listening for an incoming connection. Note:SOMAXCONN = Socket Oustanding Max Connections, if we fail to listen on listening socket...
	{
		std::string ErrorMsg = "Failed to listen on listening socket. Winsock Error:" + std::to_string(WSAGetLastError());
		MessageBoxA(NULL, ErrorMsg.c_str(), "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}
	serverPtr = this;
}



Server::~Server() {
}



bool Server::listenForNewConnection()
{
	SOCKET newConnection; //Socket to hold the client's connection

	newConnection = accept(sListen, (SOCKADDR*)&addr, &addrlen); //Accept a new connection
	if (newConnection == 0) //If accepting the client connection failed
	{
		std::cout << "Failed to accept the client's connection." << std::endl;
		return false;
	}
	else //If client connection properly accepted
	{
		std::cout << "Client Connected!" << std::endl;
		Connections[totalConnections] = newConnection; //Set socket in array to be the newest connection before creating the thread to handle this client's socket.
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)clientHandlerThread, (LPVOID)(totalConnections), NULL, NULL); //Create Thread to handle this client. The index in the socket array for this thread is the value (i).
		totalConnections += 1; //Incremenent total # of clients that have connected
		return true;
	}
}



bool Server::processPacket(int ID, Packet packettype) {
	int returnCheck = 0;

	switch (packettype)
	{
	case P_Movement:
	{
		sendMove pos;

		returnCheck = recv(Connections[ID], (char*)&pos, sizeof(pos), NULL);

		pos.plrId = ID;
		//std::cout << pos.plrId << std::endl;

		for (int i = 0; i < totalConnections; i++) {
			if (i == ID) {
				continue;
			}

			Packet movePacket = P_Movement; //create movement packet to be sent
			send(Connections[i], (char*)&movePacket, sizeof(Packet), NULL); //send movement packet
			send(Connections[i], (char*)&pos, sizeof(pos), NULL); //send movement data
		}
		break;
	}
	case P_Connection:
	{
		int cntId;
		returnCheck = recv(Connections[ID], (char*)&cntId, sizeof(cntId), NULL);
		cntId = ID;

		for (int i = 0; i < totalConnections; i++) {
			if (i == ID) {
				// Give the player that connected the list of current players
				if (totalConnections > 1) { // Someone else must be connected
					for (int j = 0; j < totalConnections; j++) { // Run through all the players
						if (j != ID) {
							Sleep(3000);
							std::cout << "Sending out player info to client: " << i << std::endl;
							Packet connectionPacket = P_Connection; //create movement packet to be sent
							send(Connections[i], (char*)&connectionPacket, sizeof(Packet), NULL); //send movement packet
							send(Connections[i], (char*)&j, sizeof(j), NULL); //send movement data
						}
					}
				}
				continue;
			}

			Packet connectionPacket = P_Connection; //create movement packet to be sent
			send(Connections[i], (char*)&connectionPacket, sizeof(Packet), NULL); //send movement packet
			send(Connections[i], (char*)&cntId, sizeof(cntId), NULL); //send movement data

			std::cout << "Connection packet received for ID: " << cntId << std::endl;
		}
		break;
	}
	default:
		std::cout << "Unknown packet received." << std::endl;
		break;
	}

	if (returnCheck == SOCKET_ERROR) {
		return false;
	}
	else {
		return true;
	}
}



void Server::clientHandlerThread(int ID) {
	while (true)
	{
		//First get the packet type
		Packet packettype;
		recv(serverPtr->Connections[ID], (char*)&packettype, sizeof(Packet), NULL); //Receive packet type from client

																		 //Once we have the packet type, process the packet
		if (!serverPtr->processPacket(ID, packettype)) //If the packet is not properly processed
			break; //break out of our client handler loop
	}
	std::cout << "Closing connection: " << ID << std::endl;
	closesocket(serverPtr->Connections[ID]); //close the socket that was being used for the client's connection
}



void Server::start()
{
	sever_thread = new std::thread(&Server::threadMain, this);
}



void Server::threadMain()
{
	//Server MyServer(1111, true); //Create server on port 1111
	for (int i = 0; i < 100; i++) //Up to 100 times...
	{
		this->listenForNewConnection(); //Accept new connection (if someones trying to connect)
	}
}



void Server::close()
{
	sever_thread->detach();
}