#include "stdafx.h"
#include "Timer_Manager.h"
#include "Event.h"
#include "Object_Manager.h"
#include "Object.h"
#include "Player.h"
#include "Monster_Base.h"

Timer_Manager* Timer_Manager::TmrMgr = nullptr;
Object_Manager* Timer_Manager::ObjMgr = nullptr;
HANDLE* Timer_Manager::iocp = nullptr;
std::atomic_int Timer_Manager::threads_number = 0;

Timer_Manager::Timer_Manager(HANDLE* const _iocp, Object_Manager* const _ObjMgr)
{
	iocp = _iocp;
	ObjMgr = _ObjMgr;
}

Timer_Manager* Timer_Manager::Create_Time_Manager(HANDLE* const _iocp, Object_Manager* const _ObjMgr)
{
	if (TmrMgr == nullptr) {
		TmrMgr = new Timer_Manager(_iocp, _ObjMgr);
		printf("[Time_Manager] - Create\n");
	}
	return TmrMgr;
}

void Timer_Manager::Process()
{
	int n_thread{threads_number++};

	printf("[Timer Manager] - Running Process thread \n");

	EVENT new_event;

	while (true) {
		new_event = Pop(n_thread);
		switch (new_event.type) {
			case EV_PLAYER_MOVE_OK:		Process_Player_Move_OK(new_event.detail);		break;
			case EV_PLAYER_Normal_Attack_OK:		Process_Player_Normal_Attack_OK(new_event.detail);		break;
			case EV_PLAYER_Spell_OK:		Process_Player_Spell_OK(new_event.detail);		break;
			case EV_MONSTER_MOVE:		Process_Monster_Move(new_event.detail);			break;
			case EV_MONSTER_MOVE_START:	Process_Monster_Move_Start(new_event.detail);	break;
			case EV_MONSTER_ATTACK:		Process_Monster_ATTACK(new_event.detail);			break;
			case EV_RESPWAN:			Process_Respwan(new_event.detail);			break;
		}
		delete new_event.detail;
	}
}

void Timer_Manager::Process_Player_Move_OK(void* const event_detail)
{
	Detail_CoolTime_OK* detail{ reinterpret_cast<Detail_CoolTime_OK*>(event_detail) };
	Type_ID id{ detail->id };

	ObjMgr->Set_Player_Move_OK(id);
}

void Timer_Manager::Process_Player_Normal_Attack_OK(void* const event_detail)
{
	Detail_CoolTime_OK* detail{ reinterpret_cast<Detail_CoolTime_OK*>(event_detail) };
	Type_ID id{ detail->id };

	ObjMgr->Set_Player_Normal_Attack_OK(id);
}

void Timer_Manager::Process_Player_Spell_OK(void* const event_detail)
{
	Detail_CoolTime_OK* detail{ reinterpret_cast<Detail_CoolTime_OK*>(event_detail) };
	Type_ID id{ detail->id };

	ObjMgr->Set_Player_Spell_OK(id);
}

void Timer_Manager::Process_Monster_Move(void* const event_detail)
{
	Detail_Monster_Move* detail{ reinterpret_cast<Detail_Monster_Move*>(event_detail) };
	Type_ID id{ detail->id };

	Object* p_object{ ObjMgr->Get_Object(id) };

	if (nullptr == p_object)			return;

	Monster_Base* p_monster{ reinterpret_cast<Monster_Base*>(p_object) };

	ZeroMemory(&p_monster->move_exover.over, sizeof(WSAOVERLAPPED));
	p_monster->move_exover.type = EX_MONSTER_MOVE;
	PostQueuedCompletionStatus(*iocp, 1, id, &p_monster->move_exover.over);
}

void Timer_Manager::Process_Monster_Move_Start(void * const event_detail)
{
	Detail_Monster_Move* detail{ reinterpret_cast<Detail_Monster_Move*>(event_detail) };
	Type_ID id{ detail->id };

	Object* p_object{ ObjMgr->Get_Object(id) };

	if (nullptr == p_object)			return;

	Monster_Base* p_monster{ reinterpret_cast<Monster_Base*>(p_object) };

	ZeroMemory(&p_monster->move_exover.over, sizeof(WSAOVERLAPPED));
	p_monster->move_exover.type = EX_MONSTER_MOVE_START;
	PostQueuedCompletionStatus(*iocp, 1, id, &p_monster->move_exover.over);
}

void Timer_Manager::Process_Monster_ATTACK(void* const event_detail)
{
	Detail_CoolTime_OK* detail{ reinterpret_cast<Detail_CoolTime_OK*>(event_detail) };
	Type_ID id{ detail->id };

	ObjMgr->Set_Monster_Attack_OK(id);
}

void Timer_Manager::Process_Respwan(void* const event_detail)
{
	Detail_Monster_Move* detail{ reinterpret_cast<Detail_Monster_Move*>(event_detail) };
	Type_ID id{ detail->id };

	if (id < MAX_PLAYER) {
		Object* p_object{ ObjMgr->Get_Object(id) };

		if (nullptr == p_object)	return;

		Expand_Overlapped* exover = new Expand_Overlapped;

		ZeroMemory(&exover->over, sizeof(WSAOVERLAPPED));
		exover->type = EX_RESPWAN_PLAYER;
		PostQueuedCompletionStatus(*iocp, 1, id, &exover->over);
	}
	else {
		Expand_Overlapped* exover = new Expand_Overlapped;

		ZeroMemory(&exover->over, sizeof(WSAOVERLAPPED));
		exover->type = EX_RESPWAN_MONSTER;
		PostQueuedCompletionStatus(*iocp, 1, id, &exover->over);
	}
}



void Timer_Manager::PQCS_Monster_Awake(const Type_ID& id)
{
	Object* p_object{ ObjMgr->Get_Object(id) };
	if (nullptr == p_object)	return;

	Expand_Overlapped* exover = new Expand_Overlapped;

	ZeroMemory(&exover->over, sizeof(WSAOVERLAPPED));
	exover->type = EX_MONSTER_AWAKE;
	PostQueuedCompletionStatus(*iocp, 1, id, &exover->over);
}


EVENT Timer_Manager::Pop(int n_thread)
{
	EVENT ret_ev;

	while (true) {

		if (false == thread_local_queue[n_thread].empty()) {

			const EVENT& ev = thread_local_queue[n_thread].top();

			if (ev.process_time < std::chrono::high_resolution_clock::now()) {
				ret_ev = ev;
				thread_local_queue[n_thread].pop();
				return ret_ev;
			}
		}
		
		if (true == concurrent_queue.try_pop(ret_ev)){

			if (std::chrono::high_resolution_clock::now() < ret_ev.process_time) {
				thread_local_queue[n_thread].push(ret_ev);
				continue;
			}
			
			else {
				return ret_ev;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}


void Timer_Manager::Push_Event_Player_Move_OK(const Player* p_player)
{
	using namespace std::chrono;

	Detail_CoolTime_OK* detail_ptr{ new Detail_CoolTime_OK(p_player->id) };

	concurrent_queue.push(EVENT(high_resolution_clock::now() + milliseconds(TIME_RECV_PLAYER_MOVE),
		EV_PLAYER_MOVE_OK, detail_ptr));
}

void Timer_Manager::Push_Event_Player_Normal_Attack_OK(const Player * const p_player)
{
	using namespace std::chrono;

	Detail_CoolTime_OK* detail_ptr{ new Detail_CoolTime_OK(p_player->id) };

	concurrent_queue.push(EVENT(high_resolution_clock::now() + milliseconds(COOLTIME_PLAYER_NORMAL_ATTACK),
		EV_PLAYER_Normal_Attack_OK, detail_ptr));
}

void Timer_Manager::Push_Event_Player_Spell_OK(const Player * const p_player)
{
	using namespace std::chrono;

	Detail_CoolTime_OK* detail_ptr{ new Detail_CoolTime_OK(p_player->id) };

	concurrent_queue.push(EVENT(high_resolution_clock::now() + milliseconds(COOLTIME_PLAYER_SPELL),
		EV_PLAYER_Spell_OK, detail_ptr));
}

void Timer_Manager::Push_Event_Monster_Move(const Monster_Base* const p_monster)
{
	using namespace std::chrono;
	Detail_Monster_Move* detail_ptr{ new Detail_Monster_Move(p_monster->id) };
	
	concurrent_queue.push(EVENT(high_resolution_clock::now() + milliseconds(S_TIME_UPDATE_OBJECT_MOVE), EV_MONSTER_MOVE, detail_ptr));
}

void Timer_Manager::Push_Event_Monster_Move_Start(const Monster_Base* const p_monster)
{
	using namespace std::chrono;
	Detail_Monster_Move* detail_ptr{ new Detail_Monster_Move(p_monster->id) };
	
	MONSTER_TYPE m_type{ p_monster->Get_Type() };

	if(m_type == M_TYPE1) 
		concurrent_queue.push(EVENT(high_resolution_clock::now() + milliseconds(TIME_MONSTER_TYPE1_NEXT_MOVE), EV_MONSTER_MOVE_START, detail_ptr));
	
	else if (m_type == M_TYPE2) 
		concurrent_queue.push(EVENT(high_resolution_clock::now() + milliseconds(S_TIME_UPDATE_OBJECT_MOVE), EV_MONSTER_MOVE, detail_ptr));
	
	else if (m_type == M_TYPE3)
		concurrent_queue.push(EVENT(high_resolution_clock::now() + milliseconds(S_TIME_UPDATE_OBJECT_MOVE), EV_MONSTER_MOVE, detail_ptr));

}

void Timer_Manager::Push_Event_Monster_type_Attack_OK(const Monster_Base* const p_monster)
{
	using namespace std::chrono;

	Detail_CoolTime_OK* detail_ptr{ new Detail_CoolTime_OK(p_monster->id) };

	MONSTER_TYPE m_type{ p_monster->Get_Type() };

	if(m_type == M_TYPE1)
		concurrent_queue.push(EVENT(high_resolution_clock::now() + milliseconds(TIME_MONSTER_TYPE1_ATTACK),EV_MONSTER_ATTACK, detail_ptr));
	
	else if (m_type == M_TYPE2)
		concurrent_queue.push(EVENT(high_resolution_clock::now() + milliseconds(TIME_MONSTER_TYPE2_ATTACK), EV_MONSTER_ATTACK, detail_ptr));
	
	else if (m_type == M_TYPE3)
		concurrent_queue.push(EVENT(high_resolution_clock::now() + milliseconds(TIME_MONSTER_TYPE3_ATTACK), EV_MONSTER_ATTACK, detail_ptr));

}

void Timer_Manager::Push_Event_Respwan(const Object* const p_object)
{
	using namespace std::chrono;
	Detail_CoolTime_OK* detail_ptr{ new Detail_CoolTime_OK(p_object->id) };

	concurrent_queue.push(EVENT(high_resolution_clock::now() + p_object->Get_respwan_time(), EV_RESPWAN, detail_ptr));
}
