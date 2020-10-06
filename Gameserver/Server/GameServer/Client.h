#pragma once

#include "Object.h"

class Client : public Object
{
private:
public:
	SOCKET			socket;
	EXOVER			exover;
	int				prev_size;
	char			packet_buf[MAX_BUF];

private:
	friend class Server_Objects;

private:
	bool CAS(std::atomic_int*, int, int) const;


public:
	//Client() = delete;

	Client() = default;

	Client(const SOCKET&, const unsigned int&);

public:
	// Get
	virtual SOCKET Get_Socket();

	// WSARECV
	void WSAReceive();

	// Packet

	// Move
	virtual bool Move(int&, int&, Direction);
	
};

