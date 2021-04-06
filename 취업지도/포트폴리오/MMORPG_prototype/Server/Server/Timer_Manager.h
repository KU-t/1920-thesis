#pragma once

#include <queue>
#include <tbb/concurrent_priority_queue.h>

struct EVENT;
class Object_Manager;
class Send_Manager;
class Object;
class Player;
class Monster_Base;

class Timer_Manager
{
private:
	tbb::concurrent_priority_queue<EVENT> concurrent_queue;
	std::priority_queue<EVENT> thread_local_queue[NUMBER_OF_THREAD__TIMER_PROCESS];

private:
	friend class Send_Manager;

public:
	static Timer_Manager* TmrMgr;
	static Object_Manager* ObjMgr;
	static HANDLE* iocp;

	Timer_Manager() = delete;
	Timer_Manager(HANDLE* const, Object_Manager* const);

public:
	static Timer_Manager* Create_Time_Manager(HANDLE* const, Object_Manager* const);
	static std::atomic_int threads_number;
private:
	void Process_Player_Move_OK(void* const);
	void Process_Player_Normal_Attack_OK(void* const);
	void Process_Player_Spell_OK(void* const);
	
	void Process_Monster_Move(void* const);
	void Process_Monster_Move_Start(void* const);
	void Process_Monster_ATTACK(void* const);
	void Process_Respwan(void* const);

public:
	void PQCS_Monster_Awake(const Type_ID&);

public:
	void Process();

	EVENT Pop(int);

	void Push_Event_Player_Move_OK(const Player* const);
	void Push_Event_Player_Normal_Attack_OK(const Player* const);
	void Push_Event_Player_Spell_OK(const Player* const);

	void Push_Event_Monster_Move(const Monster_Base* const);
	void Push_Event_Monster_Move_Start(const Monster_Base* const);

	void Push_Event_Monster_type_Attack_OK(const Monster_Base* const);

	void Push_Event_Respwan(const Object* const);
};
