#include "Object.h"

unsigned int Object::Get_id()
{
	return id;
}

int Object::Get_x()
{
	return x;
}

int Object::Get_y()
{
	return y;
}

SOCKET Object::Get_Socket()
{
	return NULL;
}

int Object::Send_Packet(EXOVER *)
{
	return 0;
}

bool Object::Move(int&, int&, Direction)
{
	return false;
}
