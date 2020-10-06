#include "Client.h"

bool Client::CAS(std::atomic_int* memory, int old_data, int new_data) const
{
	int old_value = old_data;
	int new_value = new_data;

	return std::atomic_compare_exchange_strong
	(reinterpret_cast<volatile std::atomic_int*>(memory), &old_value, new_value);
}

Client::Client(const SOCKET & _socket, const unsigned int& index)
{	
		socket = _socket;
		id = index;
		ZeroMemory(&(exover.over), sizeof(WSAOVERLAPPED));
		prev_size = 0;
		exover.type = EX_TYPE_RECEIVE;
		exover.wsabuf.buf = exover.IObuf;
		exover.wsabuf.len = MAX_BUF;
}

SOCKET Client::Get_Socket()
{
	return socket;
}

void Client::WSAReceive()
{
	prev_size = 0;
	ZeroMemory(&exover.over, sizeof(WSAOVERLAPPED));
	exover.type = EX_TYPE_RECEIVE;
	exover.wsabuf.buf = exover.IObuf;
	exover.wsabuf.len = MAX_BUF;

	DWORD flag;
	WSARecv(socket, &exover.wsabuf, 1, NULL, &flag, &exover.over, NULL);
}

bool Client::Move(int& _x, int& _y, Direction dir)
{
	int mx, my;

	switch (dir)
	{
	case CS_UP:
	{
		while (true)
		{
			mx = x; my = y;

			if (my == 0)
				return false;

			if (true == CAS(&y, my, my - 1))
			{
				_x = mx; _y = my - 1;
				return true;
			}
		}
	}
	break;
	case CS_DOWN:
	{
		while (true)
		{
			mx = x; my = y;

			if (my == WORLD_HEIGHT - 1)
				return false;

			if (true == CAS(&y, my, my + 1))
			{
				_x = mx; _y = my + 1;
				return true;
			}
		}
	}
	break;
	case CS_LEFT:
	{
		while (true)
		{
			mx = x; my = y;

			if (mx == 0)
				return false;

			if (true == CAS(&x, mx, mx - 1))
			{
				_x = mx - 1; _y = my;
				return true;
			}
		}
	}
	break;
	case CS_RIGHT:
	{
		while (true)
		{
			mx = x; my = y;

			if (my == WORLD_WIDTH - 1)
				return false;

			if (true == CAS(&x, mx, mx + 1))
			{
				_x = mx + 1; _y = my;
				return true;
			}
		}
	}
	break;
	}	
}
