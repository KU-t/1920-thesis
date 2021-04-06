#pragma once

#include "Object.h"


class Sector_Manager;
class Monster_Base;
class Send_Manager;
class Object_Manager;
class Timer_Manager;

class Player : public Object
{
public:
	Player_Expand_Overlapped	player_exover;

private:
	SOCKET	socket;

	CoolTime time__recv_move;
	CoolTime time__recv_normal_attack;
	CoolTime time__recv_spell;

	Near_set near_set;	

public:
	char*	packet_start;
	char*	recv_start;
		
public:
	Player() = delete;
	Player(const SOCKET&, const Type_ID&);

	virtual ~Player() override final;
	

private:
	friend class Object_Manager;
	friend class Send_Manager;

	void Logout();
	void Get_Near_Object_in_near_set(std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>&);
	void Update_Move(std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>&);
	virtual bool is_near(Object* const) override final;
	bool Update_position();

public:
	bool Is_Time_Recv_Move_packet();
	bool Is_Time_Recv_Move_Target_packet();
	bool Is_Time_Recv_Normal_Attack();
	bool Is_Time_Recv_SPELL();
	bool TEST_Move_packet();
	
	void Update_Near_set_Sight_in(std::vector<Object*>&);

public:
	SOCKET	Get_Socket() const;

	void recv_move_time_ok();
	void recv_normal_attack_time_ok();
	void recv_spell_time_ok();

	bool Insert_Near_set_ifn_find(Object* const);
	
	void Insert_Near_set(Object* const);

	void WSAReceive();
	void WSAReceive(CHAR*, ULONG);

	virtual bool Is_Player() override final;
	virtual Type_POS Get_Size() const override final;
	RESULT_PLAYER_MOVE Move(std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>&);
	virtual std::chrono::milliseconds Get_respwan_time() const override final;

	Type_Damage Normal_Attack(Object* const);
	Type_Damage Spell(Object* const);

	void Respwan();
};