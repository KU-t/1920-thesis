#include "Server.h"


// **************************
// ******** Error ***********
// **************************

void Server::error_display(const char* msg, int err_no) {
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcout << L"에러 " << lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
}

// ******************************************
// ******** Send_Packet_To_Client ***********
// ******************************************

void Server::Send_Packet(Client* const client, void* const buff)
{
	char *packet = reinterpret_cast<char *>(buff);
	int packetsize = packet[0];

	EXOVER *exover = new EXOVER;
	memset(exover, 0x00, sizeof(EXOVER));
	exover->type = EX_TYPE_SEND;
	memcpy(exover->IObuf, packet, packetsize);
	exover->wsabuf.buf = exover->IObuf;
	exover->wsabuf.len = packetsize;

	// ** socket이 NULL이라면?
	int ret = WSASend(client->Get_Socket(), &(exover->wsabuf), 1, 0, 0, &exover->over, 0);
	
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no) {
			error_display("[WSASend Error] : ", err_no);
			// Logout();
		}
	}
}

void Server::Send_Packet_Login(Client* const client)
{
	SC_PACKET_PLAYER_LOGIN packet;
	packet.size = sizeof(packet);
	packet.type = SC_PLAYER_LOGIN;
	packet.id = client->Get_id();
	packet.x = 7;
	packet.y = 7;

	Send_Packet(client, &packet);
}

void Server::Send_Packet_Player_Position(Client* const client_to, Client* const move_client)
{
	SC_PACKET_PLAYER_POSITION packet;
	packet.size = sizeof(packet);
	packet.type = SC_PLAYER_POSITION;
	packet.id = move_client->Get_id();
	packet.x = move_client->Get_x();
	packet.y = move_client->Get_y();

	Send_Packet(client_to, &packet);
}

void Server::Send_Packet_Player_In(ID id)
{
	/*LF::shared_ptr<Object> obj(objects.Get_Object(id));
	Client* client = reinterpret_cast<Client*>(obj.get());

	SC_PACKET_LOGIN packet;
	packet.size = sizeof(packet);
	packet.type = SC_LOGIN;
	packet.id = id;

	Send_Packet(client, &packet);*/
}

void Server::Send_Packet_Player_Out(ID id)
{
	/*LF::shared_ptr<Object> obj(objects.Get_Object(id));
	Client* client = reinterpret_cast<Client*>(obj.get());

	SC_PACKET_LOGIN packet;
	packet.size = sizeof(packet);
	packet.type = SC_LOGIN;
	packet.id = id;

	Send_Packet(client, &packet);*/
}

// *******************************
// ******** Initialize ***********
// *******************************

void Server::Init_Object()
{
	objects.Init();
}

void Server::Init_Connect()
{
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	// iocp == WSA_FLAG_OVERLAPPED
	listensocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	memset(&serveraddr, 0, sizeof(SOCKADDR_IN));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVER_PORT);
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	::bind(listensocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(SOCKADDR_IN));

	listen(listensocket, SOMAXCONN);
}

void Server::Init_CreateIOCP()
{
	iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

	// Listensock을 iocp객체에 등록
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listensocket), iocp, MAX_CLIENTS, 0);

	accept_exover.type = EX_TYPE_ACCEPT;
}

void Server::Init_Accept()
{
	acceptsocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	ZeroMemory(&accept_exover.over, sizeof(accept_exover.over));

	std::cout << "listensocket : " << listensocket << std::endl;
	std::cout << "acceptsocket : " << acceptsocket << std::endl;

	AcceptEx(listensocket, acceptsocket, accept_exover.IObuf, NULL,
		sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, NULL, &accept_exover.over);

	//std::cout << "CreateIOPort : " << acceptsocket << std::endl;
	//CreateIoCompletionPort(reinterpret_cast<HANDLE>(acceptsocket), iocp, acceptsocket, 0);
}

// ************************************
// ******** Login & Logout ************
// ************************************

void Server::Login(const EXOVER* const exover)
{
	unsigned int id = objects.Login(acceptsocket);
	
	std::cout << id << std::endl;
	
	if (id < MAX_CLIENTS)
	{
		LF::shared_ptr<Object> obj(objects.Get_Object(id));
		Client* client = reinterpret_cast<Client*>(obj.get());

		if (client == nullptr)	return;

		std::cout << "CreateIOPort(Client) : " << client->Get_Socket() << std::endl;
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(client->Get_Socket()), iocp, id, 0);
		
		//std::cout << "Login Client's socket : " << client->Get_Socket() << std::endl;

		Send_Packet_Login(client);

		client->WSAReceive();
	}
}

void Server::Logout(unsigned int id)
{

}

// *****************************
// ******** Process ************
// *****************************

void Server::Process_Accept(const EXOVER* const exover)
{
	std::cout << "ACCEPT : ";

	Login(exover);
	
	Init_Accept();
}

void Server::Process_Receive(EXOVER* const exover, ID id)
{
	std::cout << "RECEIVE : ";

	LF::shared_ptr<Object> obj(objects.Get_Object(id));
	Client* client = reinterpret_cast<Client*>(obj.get());

	if (client != nullptr)
		Process__Packet(client, exover->IObuf);


	SOCKET client_socket = client->Get_Socket();
	DWORD flags = 0;
	memset(&(exover->over), 0x00, sizeof(WSAOVERLAPPED));

	// ** client_socket가 NULL이라면?
	WSARecv(client_socket, &(exover->wsabuf), 1, 0, &flags, &(exover->over), 0);
}

void Server::Process_Send(const EXOVER* const exover)
{
	std::cout << "Send" << std::endl;
	delete exover;
}

void Server::Process__Packet(Client* const client, const void* const buff)
{
	Packet packet = reinterpret_cast<Packet>(buff);

	switch (packet[1])
	{

		case CS_UP:		case CS_DOWN:	case CS_LEFT:	case CS_RIGHT:
		{
			int clientx, clienty;
			
			if (false == client->Move(clientx, clienty, packet[1]))
				return;

			Send_Packet_Player_Position(client, client);
		}
		break;

	}
}

// **************************
// ******** public **********
// **************************

void Server::Init()
{
	Init_Object();
	Init_Connect();
	Init_CreateIOCP();
	Init_Accept();
}

void Server::Process()
{
	DWORD byte;
	ULONGLONG key64;
	PULONG_PTR keyptr = (PULONG_PTR)&key64;
	WSAOVERLAPPED *overptr;

	unsigned int id;
	EXOVER* exover;

	while (true)
	{
		std::cout << "Process Waiting..." << std::endl;
		GetQueuedCompletionStatus(iocp, &byte, keyptr, &overptr, INFINITE);
		
		id = static_cast<unsigned>(key64);
		
		if (byte == 0) {
			Logout(id);
			//continue;
		}

		exover = reinterpret_cast<EXOVER*>(overptr);

		switch (exover->type)
		{
		case EX_TYPE_ACCEPT:
			Process_Accept(exover);
			break;
		case EX_TYPE_RECEIVE:
			Process_Receive(exover, id);
			break;
		case EX_TYPE_SEND:
			Process_Send(exover);
			break;
		}
		
	}
}

// **************************
// ******** public **********
// **************************

void Server::Running()
{
	Init();
	Process();
}



