#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include <string>
#include <iostream>
#include <thread>

struct sendMove {
	float x;
	float y;
	int plrId;
};

enum Packet
{
	P_Movement,
	P_Connection
};

class Server
{
public:
	Server(int port, bool broadcastPublicly = false);
	bool listenForNewConnection();
	~Server();

	bool processPacket(int ID, Packet packettype);
	static void clientHandlerThread(int ID);

	void start();
	void close();

private:
	SOCKET Connections[100];
	int totalConnections = 0;
	std::string ip;
	int port;

	SOCKADDR_IN addr; //Address that we will bind our listening socket to
	int addrlen = sizeof(addr); //length of the address (required for accept call)
	SOCKET sListen;
	
	std::thread* sever_thread;

	void threadMain();

};

static Server* serverPtr;
