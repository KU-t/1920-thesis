#pragma once

#include <WS2tcpip.h>
#include <MSWSock.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

#include <atomic>

#include "..\..\protocol.h"

enum EXOVER_TYPE {EX_TYPE_ACCEPT, EX_TYPE_RECEIVE, EX_TYPE_SEND};

struct EXOVER
{
	WSAOVERLAPPED	over;
	WSABUF			wsabuf;
	char			IObuf[MAX_BUF];
	EXOVER_TYPE		type;
};

class Object
{
protected:
	unsigned int id;
	std::atomic_int x, y;
	int sector;

protected:
	using Direction = const int&;

public:
	// Get
	unsigned int Get_id();
	int Get_x();
	int Get_y();
	
	virtual SOCKET Get_Socket();

	// Packet
	virtual int Send_Packet(EXOVER*);

	// Move
	virtual bool Move(int&, int&, Direction);

};



