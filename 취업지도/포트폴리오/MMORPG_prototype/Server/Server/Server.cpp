#include "stdafx.h"
#include "Server.h"
#include "Login_Manager.h"
#include "Object_Manager.h"
#include "Sector_Manager.h"
#include "Terrain_Manager.h"
#include "Timer_Manager.h"
#include "Send_Manager.h"
#include "Event.h"

#include "Player.h"
#include "Monster_Base.h"

Server::Server()
{
	LgnMgr = Login_Manager::Create_Login_Manager();
	SctMgr = Sector_Manager::Create_Sector_Manager();
	TrnMgr = Terrain_Manager::Create_Terrain_Manager();
	ObjMgr = Object_Manager::Create_Object_Manager();
	TmrMgr = Timer_Manager::Create_Time_Manager(&iocp, ObjMgr);
	SndMgr = Send_Manager::Create_Send_Manager(&iocp, SctMgr);

	SctMgr->Include_Manager(ObjMgr);
	ObjMgr->Include_Manager(SctMgr, TrnMgr, TmrMgr);
	ObjMgr->Create_Monsters();
	SctMgr->Init_Monsters_Sector();
}

Server::~Server()
{
	for (auto& thread : process_threads)
		thread.join();

	closesocket(listensocket);
	WSACleanup();
}

// private

void Server::error_display(const char* const msg, int err_no) {
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcout << L"¿¡·¯ " << lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
}

void Server::Do_Connect()
{
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	listensocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	memset(&serveraddr, 0, sizeof(SOCKADDR_IN));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVER_PORT);
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	::bind(listensocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(SOCKADDR_IN));

	listen(listensocket, SOMAXCONN);

	iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listensocket), iocp, MAX_OBJECTS, 0);

	accept_exover.exover.type = EX_ACCEPT;
}

void Server::Do_Acceptex()
{
	acceptsocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	accept_exover.wsabuf.len = (ULONG)acceptsocket;
	ZeroMemory(&accept_exover.exover.over, sizeof(WSAOVERLAPPED));

	AcceptEx(listensocket, acceptsocket, accept_exover.IObuf, NULL,
		sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, NULL, &accept_exover.exover.over);
}

void Server::Do_Disconnect(Player* const p_client)
{
	closesocket(p_client->Get_Socket());
}

void Server::Do_Assemble_packet(Player* const p_client, const DWORD& _byte)
{
	unsigned char packet_size{ (unsigned char)p_client->packet_start[0] };
	char* next_recv_ptr{ p_client->recv_start + _byte };

	while (packet_size <= next_recv_ptr - p_client->packet_start) {

		Process__Packet(p_client);

		p_client->packet_start += packet_size;

		if (p_client->packet_start < next_recv_ptr)
			packet_size = p_client->packet_start[0];
		else
			break;
	}

	int left_data{ (int)(next_recv_ptr - p_client->packet_start) };

	if ((MAX_BUFFER - (next_recv_ptr - p_client->player_exover.IObuf) < MIN_BUFFER)) {
		memcpy(p_client->player_exover.IObuf, p_client->packet_start, left_data);
		next_recv_ptr = p_client->packet_start + left_data;
	}

	p_client->recv_start = next_recv_ptr;

	p_client->WSAReceive(reinterpret_cast<CHAR*>(next_recv_ptr), (ULONG)(MAX_BUFFER - (next_recv_ptr - p_client->player_exover.IObuf)));

}

void Server::Create_thread__Server_Process()
{
	for (int i{ 0 }; i < NUMBER_OF_THREAD__SERVER_PROCESS; ++i)
		process_threads.emplace_back([&]() {Process(); });
}

void Server::Create_thread__Timer_Process()
{
	for (int i{ 0 }; i < NUMBER_OF_THREAD__TIMER_PROCESS; ++i)
		process_threads.emplace_back([&]() {TmrMgr->Process(); });
}

void Server::Wait_thread__Process()
{
	for (std::thread& p_thread : process_threads)
		p_thread.join();
}

void Server::Process()
{
	printf("[Server] - Running Process thread \n");

	DWORD byte;
	ULONGLONG key64;
	PULONG_PTR keyptr{ (PULONG_PTR)&key64 };
	WSAOVERLAPPED *overptr;

	unsigned int id;
	Expand_Overlapped* p_exover;

	while (true)
	{
		GetQueuedCompletionStatus(iocp, &byte, keyptr, &overptr, INFINITE);

		id = static_cast<unsigned>(key64);

		p_exover = reinterpret_cast<Expand_Overlapped*>(overptr);

		switch (p_exover->type)
		{
			case EX_ACCEPT:				Process_Accept();						break;
			case EX_RECEIVE:			Process_Receive(id, byte);				break;
			case EX_SEND:				Process_Send(p_exover);					break;
			case EX_MONSTER_MOVE:		Process_Monster_Move(id);				break;
			case EX_MONSTER_MOVE_START:	Process_Monster_Move_Start(id);			break;
			
			case EX_MONSTER_AWAKE:	{ Process_Monster_Move_Start(id);	delete p_exover; }	break;
			case EX_RESPWAN_PLAYER:		{ Process_Player_Respwan(id);	delete p_exover; }	break;
			case EX_RESPWAN_MONSTER: { Process_Monster_Respwan(id);	delete p_exover; }	break;
		}
	}
}

void Server::Process_Accept()
{
	SOCKET socket_player{ acceptsocket };

	Do_Acceptex();

	Type_ID index{ LgnMgr->Pop() };

	printf("Login : %d - %lld \n", index, socket_player);

	if (index == -1)
		SndMgr->Notify_Login_failure(socket_player);

	else {
		Player* p_player{ ObjMgr->Login(socket_player, index) };
		Object* p_object{ reinterpret_cast<Object*>(p_player) };

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket_player), iocp, index, 0);

		std::vector<Object*> vec_near_objects;
		p_player->Update_Near_set_Sight_in(vec_near_objects);

		SctMgr->insert(p_object);

		SndMgr->Notify_Login_success(p_player, vec_near_objects);
	}
}

void Server::Process_Receive(ID_cRef id, DWORD _byte)
{
	Object* p_object{ ObjMgr->Get_Object(id) };
	if (p_object == nullptr)	return;

	Player* p_player{ reinterpret_cast<Player*>(p_object) };
	
	if (_byte == 0) {
		std::cout << "Logout : " << id << std::endl;

		Do_Disconnect(p_player);

		ObjMgr->Logout(p_player);

		SctMgr->erase(p_object);
		
		LgnMgr->Push(id);

		return;
	}

	Do_Assemble_packet(p_player, _byte);
}

void Server::Process_Send(const Expand_Overlapped* const exover)
{
	delete exover;
}

void Server::Process__Packet(Player* const p_player)
{
	const char packet_type{ p_player->packet_start[1] };
	switch (packet_type)
	{
		case CSPT_MOVE:					Process__Packet_Move(p_player);			break;
		case CSPT_MOVE_TARGET:			Process__Packet_Move_Target(p_player);			break;
		case CSPT_PLAYER_NORMAL_ATTACK:	Process__Packet_Normal_Attack(p_player);			break;
		case CSPT_PLAYER_SPELL:			Process__Packet_Spell(p_player); break;
		case T_MOVE_TARGET:				Process__Packet_TEST_Move(p_player);			break;
		default:	break;
	}
}

void Server::Process__Packet_Move(Client_Ptr p_player)
{
	if (true == p_player->Is_die()) { printf("he is dead \n"); return; }
	if (false == p_player->Is_Time_Recv_Move_packet()) return;

	TmrMgr->Push_Event_Player_Move_OK(p_player);

	std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>> vec_objects;
	
	RESULT_PLAYER_MOVE result{ p_player->Move(vec_objects) };

	if(RPM_CORRECT == result)	
		SndMgr->Notify_Player_Move(p_player, vec_objects);

	else if (RPM_MODIFY == result) {
		SndMgr->Notify_Player_Move_Modify(p_player);
		SndMgr->Notify_Player_Move(p_player, vec_objects);
	}

	else if (RPM_INCORRECT) 
		SndMgr->Notify_Player_Move_Incorrect(p_player);
}

void Server::Process__Packet_Move_Target(Client_Ptr p_player)
{
	if (true == p_player->Is_die()) { printf("he is dead \n"); return; }
	if (false == p_player->Is_Time_Recv_Move_Target_packet())	return;

	TmrMgr->Push_Event_Player_Move_OK(p_player);

	std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>> vec_objects;
	
	RESULT_PLAYER_MOVE result{ p_player->Move(vec_objects) };

	if (RPM_CORRECT == result)
		SndMgr->Notify_Player_Move_Target(p_player, vec_objects);

	else if (RPM_MODIFY == result) {
		SndMgr->Notify_Player_Move_Modify(p_player);
		SndMgr->Notify_Player_Move_Target(p_player, vec_objects);
	}

	else if (RPM_INCORRECT)
		SndMgr->Notify_Player_Move_Incorrect(p_player);
}

void Server::Process__Packet_TEST_Move(Client_Ptr p_player)
{
	if (false == p_player->TEST_Move_packet()) return;

	TmrMgr->Push_Event_Player_Move_OK(p_player);

	std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>> vec_objects;

	RESULT_PLAYER_MOVE result{ p_player->Move(vec_objects) };

	if (RPM_CORRECT == result)
		SndMgr->Notify_Player_Move_Target(p_player, vec_objects);

	else if (RPM_MODIFY == result) {
		SndMgr->Notify_Player_Move_Modify(p_player);
		SndMgr->Notify_Player_Move_Target(p_player, vec_objects);
	}

	else if (RPM_INCORRECT)
		SndMgr->Notify_Player_Move_Incorrect(p_player);
}

void Server::Process__Packet_Normal_Attack(Client_Ptr p_player)
{
	if (true == p_player->Is_die()) { printf("he is dead \n"); return; }
	if (false == p_player->Is_Time_Recv_Normal_Attack()) return;

	TmrMgr->Push_Event_Player_Normal_Attack_OK(p_player);

	CSP_PLAYER_ATTACK* p_packet{ reinterpret_cast<CSP_PLAYER_ATTACK*>(p_player->packet_start) };
	Monster_Base* p_monster{ reinterpret_cast<Monster_Base*>(ObjMgr->Get_Object(p_packet->monster_id))};
	Type_Damage damage{0};

	if (nullptr == p_monster)	return;
	if (false == p_monster->Is_exist())	return;
	if (true == p_monster->Can_Hurt()) damage = p_player->Normal_Attack(p_monster);
	
	if (-1 < damage)	SndMgr->Notify_Player_Normal_Attack(p_player, p_monster, damage);
}

void Server::Process__Packet_Spell(Client_Ptr p_player)
{
	if (true == p_player->Is_die()) { printf("he is dead \n"); return; }
		
	if (false == p_player->Is_Time_Recv_SPELL()) return;

	TmrMgr->Push_Event_Player_Spell_OK(p_player);

	CSP_PLAYER_ATTACK* p_packet{ reinterpret_cast<CSP_PLAYER_ATTACK*>(p_player->packet_start) };
	Monster_Base* p_monster{ reinterpret_cast<Monster_Base*>(ObjMgr->Get_Object(p_packet->monster_id)) };
	Type_Damage damage{ 0 };

	if (nullptr == p_monster)	return;
	if (false == p_monster->Is_exist())	return;
	if (true == p_monster->Can_Hurt()) damage = p_player->Spell(p_monster);

	if (-1 < damage)	SndMgr->Notify_Player_Spell(p_player, p_monster, damage);
}

void Server::Process_Monster_Move_Start(ID_cRef id)
{
	Monster_Base* p_monster{ reinterpret_cast<Monster_Base*>(ObjMgr->Get_Object(id)) };
	if (nullptr == p_monster)	return; 
	if (true == p_monster->Is_die())	return;

	p_monster->Awake__Set_Move_target();

	std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>> vec_objects;
	Object* p_attack_target_object{ nullptr };
	Type_Damage damage{0};

	RESULT_MONSTER_MOVE result{ p_monster->Move_or_Attack(vec_objects, p_attack_target_object,damage) };

	if (RMM_MOVE == result) {
		TmrMgr->Push_Event_Monster_Move(p_monster);
		SndMgr->Notify_Monster_Move_Target(p_monster, vec_objects);
	}

	else if (RMM_MOVE_TARGET == result) {
		TmrMgr->Push_Event_Monster_Move(p_monster);
		SndMgr->Notify_Monster_Move_Target(p_monster, vec_objects);
	}

	else if (RMM_CANT_MOVE == result) {
		if (true == p_monster->Is_sleep()) return;

		TmrMgr->Push_Event_Monster_Move_Start(p_monster);
	}

	else if (RMM_COLLISION == result) {
		TmrMgr->Push_Event_Monster_Move(p_monster);
	}

	else if (RMM_ATTACK == result) {
		TmrMgr->Push_Event_Monster_Move(p_monster);
		SndMgr->Notify_Monster_Attack(p_monster, p_attack_target_object, vec_objects, damage);
	}
}

void Server::Process_Monster_Move(ID_cRef id)
{
	Monster_Base* p_monster{ reinterpret_cast<Monster_Base*>(ObjMgr->Get_Object(id)) };
	if (nullptr == p_monster)	return;
	if (true == p_monster->Is_die())	return;
	
	std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>> vec_objects;
	Object* p_attack_target_object{nullptr};
	Type_Damage damage{ 0 };

	RESULT_MONSTER_MOVE result{ p_monster->Move_or_Attack(vec_objects, p_attack_target_object,damage) };

	if (RMM_MOVE == result) {
		TmrMgr->Push_Event_Monster_Move(p_monster);
		SndMgr->Notify_Monster_Move(p_monster, vec_objects);
	}

	else if (RMM_MOVE_TARGET == result) {
		TmrMgr->Push_Event_Monster_Move(p_monster);
		SndMgr->Notify_Monster_Move_Target(p_monster, vec_objects);
	}

	else if (RMM_CANT_MOVE == result) {
		if (true == p_monster->Is_sleep()) return;

		TmrMgr->Push_Event_Monster_Move_Start(p_monster);
	}

	else if (RMM_COLLISION == result) {
		TmrMgr->Push_Event_Monster_Move(p_monster);
	}

	else if (RMM_ATTACK == result) {
		TmrMgr->Push_Event_Monster_Move(p_monster);
		SndMgr->Notify_Monster_Attack(p_monster, p_attack_target_object, vec_objects, damage);
	}
}

void Server::Process_Player_Respwan(ID_cRef id)
{
	Player* p_player{ reinterpret_cast<Player*>(ObjMgr->Get_Object(id)) };
	if (p_player == nullptr)	return;

	p_player->Respwan();

}

void Server::Process_Monster_Respwan(ID_cRef id)
{
	Monster_Base* p_monster{ reinterpret_cast<Monster_Base*>(ObjMgr->Get_Object(id)) };

	p_monster->Respwan();
}

// public function

void Server::Running()
{
	Do_Connect();
	Do_Acceptex();

	Create_thread__Server_Process();
	Create_thread__Timer_Process();
	
	Wait_thread__Process();
}



