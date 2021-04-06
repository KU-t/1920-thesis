#pragma once
#include "Object.h"

class Login_Manager;
class Object_Manager;
class Sector_Manager;
class Terrain_Manager;
class Timer_Manager;
class Send_Manager;

class Server
{
private:
	WSADATA				wsadata;
	SOCKET				listensocket, acceptsocket;
	SOCKADDR_IN			serveraddr;
	Player_Expand_Overlapped	accept_exover;

	HANDLE iocp;

	Login_Manager*		LgnMgr;
	Object_Manager*		ObjMgr;
	Sector_Manager*		SctMgr;
	Terrain_Manager*	TrnMgr;
	Timer_Manager*		TmrMgr;
	Send_Manager*		SndMgr;

	std::vector<std::thread> process_threads;

private:
	using Vector_objs			= std::vector<Object*>;
	using Objects_vec_Ptr		= std::vector<Object*>* const;
	using Vector_pair_objs_move	= std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>;
	using Vector_pair_objs_move_Ref	= std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>&;

	using Object_cPtr		= const Object* const;
	using Object_Ptr		= Object * const;

	using Client_cPtr	= const Player* const;
	using Client_Ptr	= Player * const;

	using Buffer_Ptr	= void* const;
	using Socket_cRef	= const SOCKET&;
	using ID_cRef		= const Type_ID&;

	using Duration_Time = const std::chrono::duration<float, std::milli>&;

public:
	Server();
	~Server();

private:

	void error_display(const char* const, int);

	void Do_Connect();
	void Do_Acceptex();
	void Do_Disconnect(Player* const);
	void Do_Assemble_packet(Player* const, const DWORD&);

	void Create_thread__Server_Process();
	void Create_thread__Timer_Process();
	void Wait_thread__Process();

	void Process();
	void Process_Accept();
	void Process_Receive(ID_cRef, DWORD);
	void Process_Send(const Expand_Overlapped* const);
	void Process__Packet(Player* const);
	void Process__Packet_Move(Client_Ptr);
	void Process__Packet_Move_Target(Client_Ptr);
	void Process__Packet_TEST_Move(Client_Ptr);
	void Process__Packet_Normal_Attack(Client_Ptr);
	void Process__Packet_Spell(Client_Ptr);
	void Process_Monster_Move_Start(ID_cRef);
	void Process_Monster_Move(ID_cRef);
	void Process_Player_Respwan(ID_cRef);
	void Process_Monster_Respwan(ID_cRef);

public:
	void Running();

};

