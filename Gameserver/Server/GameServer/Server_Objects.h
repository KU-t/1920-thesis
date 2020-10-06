#pragma once

#include <tbb/concurrent_queue.h>
//#include <tbb/concurrent_hash_map.h>

#include "Client.h"
#include "Monster.h"

#include "Lock-Free_Smart_Pointer.h"
#include "Server_value.h"

class Server_Objects
{
private:
	tbb::concurrent_queue<unsigned int> client_login_queue;
//
public:
	LF::shared_ptr<Object> objects[MAX_CLIENTS];

private:
	void Init__Queue();
	
public:
	// Get
	LF::shared_ptr<Object>& Get_Object(const unsigned int&);


public:
	void Init();

	unsigned int Login(const SOCKET);
	unsigned int Get_index();

	void Logout();

};

