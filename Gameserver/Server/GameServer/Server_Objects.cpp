#include "Server_Objects.h"

void Server_Objects::Init()
{
	Init__Queue();
}

void Server_Objects::Init__Queue()
{
	for (unsigned int i = 0; i < MAX_CLIENTS; ++i)
		client_login_queue.push(i);
}

unsigned int Server_Objects::Login(const SOCKET acceptsocket)
{
	unsigned int index = Get_index();
	
	if (index < MAX_CLIENTS)
	{
		Object* newclient = reinterpret_cast<Object*>(new Client(acceptsocket, index));
		objects[index] = newclient;
	}

	return index;
}

unsigned int Server_Objects::Get_index()
{
	int i = 0;
	unsigned int index = MAX_CLIENTS + 1;

	while (i < TRY_LOGIN) {
		client_login_queue.try_pop(index);

		if (index < MAX_CLIENTS)
			return index;
	}

	return MAX_CLIENTS + 1;
}

void Server_Objects::Logout()
{
}

LF::shared_ptr<Object>& Server_Objects::Get_Object(const unsigned int& id)
{
	return objects[id];
}
