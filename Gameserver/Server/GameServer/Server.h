#pragma once

#include <WS2tcpip.h>
#include <MSWSock.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

#include <iostream>

#include "Server_Objects.h"

class Server
{
private:
	WSADATA			wsadata;
	SOCKET			listensocket, acceptsocket;
	SOCKADDR_IN		serveraddr;

	EXOVER			accept_exover;

	HANDLE iocp;
	
	Server_Objects	objects;

	using Packet = const char* const;
	using ID = const unsigned int&;

private:
	// Error
	void error_display(const char*, int);

	// Send_Packet_To_Client
	void Send_Packet(Client* const, void* const);
	void Send_Packet_Login(Client* const);
	void Send_Packet_Player_Position(Client* const, Client* const);
	void Send_Packet_Player_In(ID);
	void Send_Packet_Player_Out(ID);

private:
	
	// Initialize
	void Init_Object();
	void Init_Connect();
	void Init_CreateIOCP();
	void Init_Accept();
	
	// Login & Logout
	void Login(const EXOVER* const);
	void Logout(unsigned int);

	// Process
	void Process_Accept(const EXOVER* const);
	void Process_Receive(EXOVER* const, ID);
	void Process_Send(const EXOVER* const);
	
	void Process__Packet(Client* const, const void* const);

private:
	
	void Init();
	void Process();
	
public:

	void Running();
};

