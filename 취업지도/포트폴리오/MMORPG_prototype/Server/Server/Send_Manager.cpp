#include "stdafx.h"
#include "Send_Manager.h"
#include "Sector_Manager.h"
#include "Object.h"

#include "Player.h"
#include "Monster_Base.h"

Send_Manager* Send_Manager::SndMgr = nullptr;
Sector_Manager* Send_Manager::SctMgr = nullptr;
HANDLE* Send_Manager::iocp = nullptr;

void Send_Manager::Send_Packet(const Player* const p_player, void* const p_buf, bool check)
{
	char* packet{ reinterpret_cast<char *>(p_buf) };
	int packetsize{ packet[0] };

	Player_Expand_Overlapped* p_exover{ new Player_Expand_Overlapped };
	memset(p_exover, 0x00, sizeof(Player_Expand_Overlapped));
	p_exover->exover.type = EX_SEND;
	memcpy(p_exover->IObuf, packet, packetsize);
	p_exover->wsabuf.buf = reinterpret_cast<CHAR*>(p_exover->IObuf);
	p_exover->wsabuf.len = packetsize;

	if (check || p_player->Is_exist()) {

		int ret{ WSASend(p_player->Get_Socket(), &(p_exover->wsabuf), 1, 0, 0, &p_exover->exover.over, 0) };

		if (0 != ret) {
			int err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no)
				printf("[Send_Manager] - Send client Error %lld \n", p_player->Get_Socket());
		}
	}
}

void Send_Manager::Send_Login_failure(const SOCKET& client_socket)
{
	SCP_LOGIN_FAILURE packet;
	packet.size = sizeof(packet);
	packet.type = SCPT_LOGIN_FAILURE;

	char* p_packet{ reinterpret_cast<char*>(&packet) };
	int packetsize{ p_packet[0] };

	Player_Expand_Overlapped* p_exover{ new Player_Expand_Overlapped };
	memset(p_exover, 0x00, sizeof(Player_Expand_Overlapped));
	p_exover->exover.type = EX_SEND;
	memcpy(p_exover->IObuf, p_packet, packetsize);
	p_exover->wsabuf.buf = reinterpret_cast<CHAR*>(p_exover->IObuf);
	p_exover->wsabuf.len = packetsize;

	int ret{ WSASend(client_socket, &(p_exover->wsabuf), 1, 0, 0, &p_exover->exover.over, 0) };

	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			printf("[Send_Manager] - Send client Error %lld \n", client_socket);
	}
}

void Send_Manager::Send_Login_success(const Player* const p_player)
{
	SCP_LOGIN_SUCCESS packet;
	packet.size = sizeof(packet);
	packet.type = SCPT_LOGIN_SUCCESS;

	packet.id = p_player->id;
	packet.x = p_player->x;
	packet.y = p_player->y;
	packet.vel = p_player->vel;
	packet.Hp = p_player->Hp;
	packet.str = p_player->str;

	Send_Packet(p_player, &packet, true);
}

void Send_Manager::Send_World_in_Player(Player* const p_world_in_player, std::vector<Object*>& vec_objects)
{
	SCP_SIGHT_IN Packet_world_in_player{ Create_Packet_Sight_in(p_world_in_player) };
	SCP_SIGHT_IN Packet_other_player{ Create_Packet_Sight_in() };

	for (Object* const p_object : vec_objects) {
		
		Change_Packet_data(Packet_other_player, p_object);

		if (true == p_object->Is_Player()) {
			Player* p_player{ reinterpret_cast<Player*>(p_object) };

			Send_Packet(p_world_in_player, &Packet_other_player, true);
			Send_Packet(p_player, &Packet_world_in_player, false);
		}

		else {
			Send_Packet(p_world_in_player, &Packet_other_player, true);
		}
	}
}



void Send_Manager::Notify_Login_failure(const SOCKET& socket)
{
	Send_Login_failure(socket);
}

void Send_Manager::Notify_Login_success
	(Player* const p_player, std::vector<Object*>& vec_near_objects)
{
	Send_Login_success(p_player);
	Send_World_in_Player(p_player, vec_near_objects);
}

void Send_Manager::Notify_Player_Move
	(Player* const p_move_player, const std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>& vec_objects)
{
	Object* p_move_object{ reinterpret_cast<Object*>(p_move_player) };

	SCP_MOVE packet__player_move{ Create_Packet_Move(p_move_object) };
	SCP_SIGHT_IN packet__player_in{ Create_Packet_Sight_in(p_move_object) };
	SCP_SIGHT_OUT packet__player_out{ Create_Packet_Sight_out(p_move_object) };

	SCP_SIGHT_IN packet__other_player_in{ Create_Packet_Sight_in() };
	SCP_SIGHT_OUT packet__other_player_out{ Create_Packet_Sight_out() };
	
	for (const std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>& vec_element : vec_objects) {
		Object* p_object = vec_element.first;


		if (vec_element.second == RMOT_UNEXIST) {
			Change_Packet_data(packet__other_player_out, p_object);
			Send_Packet(p_move_player, &packet__other_player_out, true);
		}

		else if (true == p_object->Is_Player()) {
			Player* p_player = reinterpret_cast<Player*>(p_object);
		
			if (vec_element.second == RMOT_PLAYER_NEAR) 
				Send_Packet(p_player, &packet__player_move, false);
			
			else if (vec_element.second == RMOT_PLAYER_SIGHT_IN) {
				Change_Packet_data(packet__other_player_in, p_player);
				Send_Packet(p_player, &packet__player_in, false);
				Send_Packet(p_move_player, &packet__other_player_in, true);
			}
			else if (vec_element.second == RMOT_PLAYER_SIGHT_OUT) {
				Change_Packet_data(packet__other_player_out, p_player);
				Send_Packet(p_player, &packet__player_out, false);
				Send_Packet(p_move_player, &packet__other_player_out, true);
			}
		}

		else {
			if (vec_element.second == RMOT_PLAYER_NEAR) continue;

			if (vec_element.second == RMOT_OBJECT_SIGHT_IN) {
				Change_Packet_data(packet__other_player_in, p_object);
				Send_Packet(p_move_player, &packet__other_player_in, true);
			}
			else if (vec_element.second == RMOT_OBJECT_SIGHT_OUT) {
				Change_Packet_data(packet__other_player_in, p_object);
				Send_Packet(p_move_player, &packet__other_player_in, true);
			}
		}
	}
}

void Send_Manager::Notify_Player_Move_Modify(Player* const p_move_player)
{
	Object* p_move_object{ reinterpret_cast<Object*>(p_move_player) };
	SCP_MOVE packet__player_move{ Create_Packet_Move(p_move_object) };
	
	Send_Packet(p_move_player, &packet__player_move, true);
}

void Send_Manager::Notify_Player_Move_Incorrect(Player* const p_move_player)
{
	Object* p_move_object{ reinterpret_cast<Object*>(p_move_player) };
	SCP_MOVE_TARGET packet__player_move_target{ Create_Packet_Move_Target(p_move_object) };
	
	Send_Packet(p_move_player, &packet__player_move_target, true);
}

void Send_Manager::Notify_Player_Move_Target
	(Player* const p_move_player, const std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>& vec_objects)
{
	Object* p_move_object{reinterpret_cast<Object*>(p_move_player)};

	SCP_MOVE_TARGET packet__player_move_target{ Create_Packet_Move_Target(p_move_object) };
	SCP_SIGHT_IN packet__player_in{ Create_Packet_Sight_in(p_move_object) };
	SCP_SIGHT_OUT packet__player_out{ Create_Packet_Sight_out(p_move_object) };

	SCP_SIGHT_IN packet__other_player_in{ Create_Packet_Sight_in() };
	SCP_SIGHT_OUT packet__other_player_out{ Create_Packet_Sight_out() };
	
	for (const std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>& vec_element : vec_objects) {
		Object* p_object = vec_element.first;

		if (vec_element.second == RMOT_UNEXIST) {
			Change_Packet_data(packet__other_player_out, p_object);
			Send_Packet(p_move_player, &packet__other_player_out, true);
		}

		else if (true == p_object->Is_Player()) {
			Player* p_player{ reinterpret_cast<Player*>(p_object) };

			if (vec_element.second == RMOT_PLAYER_NEAR) {
				Send_Packet(p_player, &packet__player_move_target, false);
			}
			else if (vec_element.second == RMOT_PLAYER_SIGHT_IN) {
				Change_Packet_data(packet__other_player_in, p_player);

				Send_Packet(p_player, &packet__player_in, false);
				Send_Packet(p_move_player, &packet__other_player_in, true);
			}
			else if (vec_element.second == RMOT_PLAYER_SIGHT_OUT) {
				Change_Packet_data(packet__other_player_out, p_player);

				Send_Packet(p_player, &packet__player_out, false);
				Send_Packet(p_move_player, &packet__other_player_out, true);
			}
		}

		else {
			if (vec_element.second == RMOT_PLAYER_NEAR) continue;

			if (vec_element.second == RMOT_OBJECT_SIGHT_IN) {
				Change_Packet_data(packet__other_player_in, p_object);

				Send_Packet(p_move_player, &packet__other_player_in, true);
			}
			else if (vec_element.second == RMOT_OBJECT_SIGHT_OUT) {
				Change_Packet_data(packet__other_player_out, p_object);

				Send_Packet(p_move_player, &packet__other_player_out, true);
			}
		}
	}
}

void Send_Manager::Notify_Player_Normal_Attack(Player* const p_player, Object* const p_monster, const Type_Damage& damage)
{
	CSP_PLAYER_ATTACK* p_recv_packet
	{ reinterpret_cast<CSP_PLAYER_ATTACK*>(p_player->packet_start) };
	
	SCP_PLAYER_ATTACK_RESULT packet__player_normal_attack_result
	{ Create_Packet_Normal_Attack_Result(p_monster,damage) };

	Send_Packet(p_player, &packet__player_normal_attack_result, true);

	SCP_PLAYER_ATTACK packet__player_normal_attack
	{ Create_Packet_Normal_Attack(p_player,p_monster,damage) };

	for (Object* const p_near_set_object : p_player->near_set) {
		if (true == p_near_set_object->Is_Player()) {
			Player* p_near_set_player{ reinterpret_cast<Player*>(p_near_set_object) };
			Send_Packet(p_near_set_player, &packet__player_normal_attack, false);
		}
	}
}

void Send_Manager::Notify_Player_Spell(Player* const p_player, Object* const p_monster, const Type_Damage& damage)
{
	CSP_PLAYER_ATTACK* p_recv_packet
	{ reinterpret_cast<CSP_PLAYER_ATTACK*>(p_player->packet_start) };

	SCP_PLAYER_ATTACK_RESULT packet__player_spell_result
	{ Create_Packet_Spell_Result(p_monster,damage) };

	Send_Packet(p_player, &packet__player_spell_result, true);

	SCP_PLAYER_ATTACK packet__player_spell
	{ Create_Packet_Spell(p_player,p_monster,damage) };

	for (Object* const p_near_set_object : p_player->near_set) {
		if (true == p_near_set_object->Is_Player()) {
			Player* p_near_set_player{ reinterpret_cast<Player*>(p_near_set_object) };
			Send_Packet(p_near_set_player, &packet__player_spell, false);
		}
	}
}

void Send_Manager::Notify_Monster_Move
	(Monster_Base* const p_move_monster, const std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>& vec_objects)
{
	Object* p_move_object{ reinterpret_cast<Object*>(p_move_monster) };

	SCP_MOVE packet__monster_move{ Create_Packet_Move(p_move_object) };

	SCP_SIGHT_IN packet__monster_in{ Create_Packet_Sight_in(p_move_object) };

	for (const std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>& vec_element : vec_objects) {
		Player* p_player{ reinterpret_cast<Player*>(vec_element.first) };

		if (vec_element.second == RMOT_PLAYER_NEAR) 
			Send_Packet(p_player, &packet__monster_move, false);
		
		else if (vec_element.second == RMOT_PLAYER_SIGHT_IN) 
			Send_Packet(p_player, &packet__monster_in, false);
		
	}
}

void Send_Manager::Notify_Monster_Move_Target
	(Monster_Base* const p_move_monster, const std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>& vec_objects)
{
	Object* p_move_object{ reinterpret_cast<Object*>(p_move_monster) };

	SCP_MOVE_TARGET packet__monster_move_target{ Create_Packet_Move_Target(p_move_object) };

	SCP_SIGHT_IN packet__monster_in{ Create_Packet_Sight_in(p_move_object) };

	for (const std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>& vec_element : vec_objects) {
		Player* p_player{ reinterpret_cast<Player*>(vec_element.first) };

		if (vec_element.second == RMOT_PLAYER_NEAR)
			Send_Packet(p_player, &packet__monster_move_target, false);

		else if (vec_element.second == RMOT_PLAYER_SIGHT_IN)
			Send_Packet(p_player, &packet__monster_in, false);

	}
}

void Send_Manager::Notify_Monster_Attack
	(Monster_Base* const p_attack_monster, Object* const p_target_object, const std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>& vec_objects, const Type_Damage& damage)
{
	Object* p_move_object{ reinterpret_cast<Object*>(p_attack_monster) };

	SCP_MONSTER_ATTACK packet__monster_attack{ Create_Packet_Monster_Attack(p_attack_monster, p_target_object,damage) };

	SCP_SIGHT_IN packet__monster_in{ Create_Packet_Sight_in(p_move_object) };

	for (const std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>& vec_element : vec_objects) {
		Player* p_player{ reinterpret_cast<Player*>(vec_element.first) };

		if (vec_element.second == RMOT_PLAYER_NEAR)
			Send_Packet(p_player, &packet__monster_attack, false);

		else if (vec_element.second == RMOT_PLAYER_SIGHT_IN)
			Send_Packet(p_player, &packet__monster_in, false);

	}
}

Send_Manager::Send_Manager(HANDLE* const _iocp, Sector_Manager* const _SctMgr)
{
	iocp = _iocp;
	SctMgr = _SctMgr;
}

Send_Manager * Send_Manager::Create_Send_Manager(HANDLE* const _iocp, Sector_Manager* const _SctMgr)
{
	if (SndMgr == nullptr) {
		SndMgr = new Send_Manager(_iocp, _SctMgr);
		printf("[Send_Manager] - Create \n");
	}
	return SndMgr;
}



SCP_SIGHT_IN Send_Manager::Create_Packet_Sight_in()
{
	SCP_SIGHT_IN packet;
	packet.size = sizeof(SCP_SIGHT_IN);
	packet.type = SCPT_SIGHT_IN;
	return packet;
}

SCP_SIGHT_IN Send_Manager::Create_Packet_Sight_in(const Object* const p_object)
{
	SCP_SIGHT_IN packet;
	packet.size = sizeof(SCP_SIGHT_IN);
	packet.type = SCPT_SIGHT_IN;
	packet.id = p_object->id;
	packet.x = p_object->x;
	packet.y = p_object->y;
	packet.target_x = p_object->target_x;
	packet.target_y = p_object->target_y;
	packet.vel = p_object->vel;
	Type_Hp p_obect_hp{ p_object->Hp };
	packet.Hp = p_obect_hp;

	return packet;
}

SCP_SIGHT_OUT Send_Manager::Create_Packet_Sight_out()
{
	SCP_SIGHT_OUT packet;
	packet.size = sizeof(SCP_SIGHT_OUT);
	packet.type = SCPT_SIGHT_OUT;

	return packet;
}

SCP_SIGHT_OUT Send_Manager::Create_Packet_Sight_out(const Object* const p_object)
{
	SCP_SIGHT_OUT packet;
	packet.size = sizeof(SCP_SIGHT_OUT);
	packet.type = SCPT_SIGHT_OUT;
	packet.id = p_object->id;
	return packet;
}

SCP_PLAYER_ATTACK Send_Manager::Create_Packet_Normal_Attack(const Player* const p_player, const Object* const p_monster, const Type_Damage& damage)
{
	SCP_PLAYER_ATTACK packet;
	packet.size = sizeof(SCP_PLAYER_ATTACK);

	if (true == p_monster->Is_exist())	packet.type = SCPT_PLAYER_NORMAL_ATTACK;
	else packet.type = SCPT_PLAYER_NORMAL_ATTACK_KILL;

	packet.player_id = p_player->id;
	packet.monster_id = p_monster->id;
	packet.damage = damage;

	return packet;
}

SCP_PLAYER_ATTACK Send_Manager::Create_Packet_Spell(const Player* const p_player, const Object* const p_monster, const Type_Damage& damage)
{
	SCP_PLAYER_ATTACK packet;
	packet.size = sizeof(SCP_PLAYER_ATTACK);
	
	if (true == p_monster->Is_exist())	packet.type = SCPT_PLAYER_SPELL;
	else packet.type = SCPT_PLAYER_SPELL_KILL;

	packet.player_id = p_player->id;
	packet.monster_id = p_monster->id;
	packet.damage = damage;

	return packet;
}

SCP_PLAYER_ATTACK_RESULT Send_Manager::Create_Packet_Normal_Attack_Result(const Object* const p_monster, const Type_Damage& damage)
{
	SCP_PLAYER_ATTACK_RESULT packet;
	packet.size = sizeof(SCP_PLAYER_ATTACK_RESULT);

	packet.type = SCPT_PLAYER_NORMAL_ATTACK_RESULT;

	if(true == p_monster->Is_exist())	packet.type = SCPT_PLAYER_NORMAL_ATTACK_RESULT;
	else packet.type = SCPT_PLAYER_NORMAL_ATTACK_RESULT_KILL;

	packet.monster_id = p_monster->id;
	packet.damage = damage;

	return packet;
}

SCP_PLAYER_ATTACK_RESULT Send_Manager::Create_Packet_Spell_Result(const Object* const p_monster, const Type_Damage& damage)
{
	SCP_PLAYER_ATTACK_RESULT packet;
	packet.size = sizeof(SCP_PLAYER_ATTACK_RESULT);
	
	packet.type = SCPT_PLAYER_SPELL_RESULT;

	if (true == p_monster->Is_exist())	packet.type = SCPT_PLAYER_SPELL_RESULT;
	else packet.type = SCPT_PLAYER_SPELL_RESULT_KILL;

	packet.monster_id = p_monster->id;
	packet.damage = damage;

	return packet;
}

SCP_MONSTER_ATTACK Send_Manager::Create_Packet_Monster_Attack(const Monster_Base* const p_monster, const Object * const p_player, const Type_Damage& damage)
{
	SCP_MONSTER_ATTACK packet;
	packet.size = sizeof(SCP_MONSTER_ATTACK);

	MONSTER_TYPE type{ p_monster->Get_Type() };
	if (M_TYPE1 == type)		packet.type = SCPT_MONSTER_TYPE1_ATTACK;
	else if (M_TYPE2 == type)	packet.type = SCPT_MONSTER_TYPE2_ATTACK;
	else if (M_TYPE3 == type)	packet.type = SCPT_MONSTER_TYPE3_ATTACK;

	packet.x = p_monster->x;
	packet.y = p_monster->y;
	packet.target_x = p_monster->target_x;
	packet.target_y = p_monster->target_y;
	packet.player_id = p_player->id;
	packet.monster_id = p_monster->id;
	packet.damage = damage;

	return packet;
}

SCP_MOVE Send_Manager::Create_Packet_Move(const Object* const p_object)
{
	SCP_MOVE packet;
	packet.size = sizeof(SCP_MOVE);
	packet.type = SCPT_MOVE;
	packet.id = p_object->id;
	packet.x = p_object->x;
	packet.y = p_object->y;
	return packet;
}

SCP_MOVE_TARGET Send_Manager::Create_Packet_Move_Target(const Object* const p_object)
{
	SCP_MOVE_TARGET packet;
	packet.size = sizeof(SCP_MOVE_TARGET);
	packet.type = SCPT_MOVE_TARGET;
	packet.id = p_object->id;
	packet.x = p_object->x;
	packet.y = p_object->y;
	packet.target_x = p_object->target_x;
	packet.target_y = p_object->target_y;
	return packet;
}

void Send_Manager::Change_Packet_data(SCP_SIGHT_IN& packet, const Object* const p_object)
{
	packet.id = p_object->id;
	packet.x = p_object->x;
	packet.y = p_object->y;
	packet.target_x = p_object->target_x;
	packet.target_y = p_object->target_y;
	packet.vel = p_object->vel;
	packet.Hp = p_object->Hp;
}

void Send_Manager::Change_Packet_data(SCP_SIGHT_OUT& packet, const Object* const p_object)
{
	packet.id = p_object->id;
}
