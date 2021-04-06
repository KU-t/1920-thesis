#include "stdafx.h"
#include "Player.h"
#include "Terrain_Manager.h"
#include "Sector_Manager.h"
#include "Monster_Base.h"
#include "Timer_Manager.h"
#include "Object_Manager.h"

Player::Player(const SOCKET & _socket, const Type_ID& index)
{
	packet_start = player_exover.IObuf;
	recv_start = player_exover.IObuf;

	socket = _socket;
	id = index;

	exist = true;
	die = false;

	pred_move_time = std::chrono::high_resolution_clock::now();

	/*
	Type_POS px, py;
	while (true) {
		px = (float)(rand() % (int)WORLD_MAP_SIZE_WIDTH);
		py = (float)(rand() % (int)WORLD_MAP_SIZE_HEIGHT);

		if (true == TrnMgr->Can_Create_Object(px, py))
			break;
	}
	*/

	target_x = x = 459.f;
	target_y = y = 204.f;

	vel = 3.f;
	Hp = 1000;
	str = 3;

	WSAReceive();
}

Player::~Player()
{}

void Player::Get_Near_Object_in_near_set
	(std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>& vec_objects)
{
	near_set.R_lock();

	for (Object* p_other_object : near_set) {
		
		if (false == p_other_object->Is_exist()) 
			vec_objects.emplace_back(std::make_pair(p_other_object, RMOT_UNEXIST));

		else if (true == p_other_object->Is_Player()) {

			if (true == is_near(p_other_object))
				vec_objects.emplace_back(std::make_pair(p_other_object, RMOT_PLAYER_NEAR));

			else
				vec_objects.emplace_back(std::make_pair(p_other_object, RMOT_PLAYER_SIGHT_OUT));
		}

		else {
			if (true == is_near(p_other_object)) 
				vec_objects.emplace_back(std::make_pair(p_other_object, RMOT_OBJECT_NEAR));
			
			else 
				vec_objects.emplace_back(std::make_pair(p_other_object, RMOT_OBJECT_SIGHT_OUT));
		}
	}

	near_set.R_unlock();
}

void Player::Update_Move(std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>& vec_objects)
{
	std::vector<Sector_Base*> vec_near_sector{ SctMgr->Get_near_sector(x, y) };
	std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>::iterator iter;
	
	for (Sector_Base* const sector : vec_near_sector) {

		sector->R_lock();

		for (Object* const p_sector_object : *sector) {
			
			if (false == p_sector_object->Is_exist()) continue;

			if (true == is_near(p_sector_object)) {
				if (true == p_sector_object->Is_Player()) {

					for (iter = vec_objects.begin(); iter != vec_objects.end(); ++iter) {
						if (iter->second != RMOT_PLAYER_NEAR)	continue;

						Object* p_vec_player{ iter->first };

						if (p_vec_player == p_sector_object)	break;
					}

					if (iter == vec_objects.end()) {
						vec_objects.emplace_back(std::make_pair(p_sector_object, RMOT_PLAYER_SIGHT_IN));
					}
				}

				else {
					
					for (iter = vec_objects.begin(); iter != vec_objects.end(); ++iter) {
						if (iter->second != RMOT_OBJECT_NEAR)	continue;

						Object* p_vec_object{ iter->first };

						if (p_vec_object == p_sector_object)	
							break;
					}

					if (iter == vec_objects.end()) {
						vec_objects.emplace_back(std::make_pair(p_sector_object, RMOT_OBJECT_SIGHT_IN));
					}
				}
			}
		}

		sector->R_unlock();
	}

	for (const std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>& vec_element : vec_objects) {
		Object* p_object = vec_element.first;

		if (vec_element.second == RMOT_UNEXIST) continue;

		if (true == p_object->Is_Player()) {
			Player* p_player{ reinterpret_cast<Player*>(p_object) };
			p_player->near_set.insert(this);
		}

		else {
			if (vec_element.second == RMOT_OBJECT_SIGHT_IN || vec_element.second == RMOT_OBJECT_NEAR)
			{
				Monster_Base* p_monster{reinterpret_cast<Monster_Base*>(vec_element.first)};
				if (true == p_monster->Awake()) {
					printf("awake\n");
					TmrMgr->PQCS_Monster_Awake(p_monster->id);
				}
			}
		}
	}

	near_set.Update(vec_objects);
}

bool Player::is_near(Object * const other_obj)
{
	if (OBJECT_NEAR_WIDTH < x - other_obj->x)	return false;
	if (OBJECT_NEAR_WIDTH < other_obj->x - x)	return false;
	if (OBJECT_NEAR_HEIGHT < y - other_obj->y)	return false;
	if (OBJECT_NEAR_HEIGHT < other_obj->y - y)	return false;
	if (other_obj == this)						return false;
	return true;
}

bool Player::Update_position()
{
	using namespace std;
	using namespace chrono;
	
	high_resolution_clock::time_point curr_time{ high_resolution_clock::now() };
	duration<float, milli> time{ duration_cast<milliseconds>(curr_time - pred_move_time) };
	pred_move_time = curr_time;

	CSP_MOVE* p_move_packet{ reinterpret_cast<CSP_MOVE*>(packet_start) };

	float length_packet_x{ p_move_packet->x - x };
	float length_packet_y{ p_move_packet->y - y };
	float length_packet{ hypotf(length_packet_x, length_packet_y) };

	float length_rate{ vel * 0.001f * TIME_MOVE_INTERPOLATION };

	if (length_packet < length_rate) {
		x = p_move_packet->x;
		y = p_move_packet->y;
		return true;
	}

	float length_target_x{ target_x - x };
	float length_target_y{ target_y - y };
	float length_target{ hypotf(length_target_x, length_target_y) };

	length_rate = vel * 0.001f * time.count();

	if (length_rate < length_target) {
		x += length_rate * length_target_x / length_target;
		x += length_rate * length_target_y / length_target; 
	}

	else {
		x = target_x;
		y = target_y;
	}
		
	return false;
}

void Player::Logout()
{
	exist = false;
	near_set.clear();
}

void Player::Update_Near_set_Sight_in(std::vector<Object*>& vec_near_objects)
{
	std::vector<Sector_Base*> vec_near_sector{ SctMgr->Get_near_sector(x, y) };
	
	for (Sector_Base* const sector : vec_near_sector) {

		sector->R_lock();

		for (Object* const p_object : *sector) {

			if (false == p_object->Is_exist())	continue;

			if (true == is_near(p_object)) {
				vec_near_objects.emplace_back(p_object);
			}
		}
		
		sector->R_unlock();
	}
	
	for (Object* const p_object : vec_near_objects) {
		if (true == p_object->Is_Player()){
			Player* p_player{ reinterpret_cast<Player*>(p_object) };
			p_player->near_set.insert(this);
		}
		/*else {
			Monster_Base* p_monster{ reinterpret_cast<Monster_Base*>(p_object) };
			if (true == p_monster->Awake()) TmrMgr->PQCS_Monster_Awake(p_monster->id);
		}*/
	}

	near_set.insert(vec_near_objects);
}

void Player::WSAReceive()
{
	ZeroMemory(&player_exover.exover.over, sizeof(WSAOVERLAPPED));
	player_exover.exover.type = EX_RECEIVE;
	player_exover.wsabuf.buf = reinterpret_cast<CHAR*>(player_exover.IObuf);
	player_exover.wsabuf.len = MAX_BUFFER;

	DWORD flag = 0;
	WSARecv(socket, &player_exover.wsabuf, 1, NULL, &flag, &player_exover.exover.over, NULL);
}

void Player::WSAReceive(CHAR* start_ptr, ULONG size)
{
	ZeroMemory(&player_exover.exover.over, sizeof(WSAOVERLAPPED));
	player_exover.exover.type = EX_RECEIVE;
	player_exover.wsabuf.buf = start_ptr;
	player_exover.wsabuf.len = size;

	DWORD flag = 0;
	WSARecv(socket, &player_exover.wsabuf, 1, NULL, &flag, &player_exover.exover.over, NULL);
}

bool Player::Is_Time_Recv_Move_packet()
{
	if (false == time__recv_move.Is_cooltime())	return false;
	return time__recv_move.Reset_cooltime();
}

bool Player::Is_Time_Recv_Move_Target_packet()
{
	if (false == time__recv_move.Is_cooltime())	return false;
	if (false == time__recv_move.Reset_cooltime())	return false;

	CSP_MOVE_TARGET* packet{ reinterpret_cast<CSP_MOVE_TARGET*>(packet_start) };

	target_x = packet->target_x;
	target_y = packet->target_y;

	pred_move_time = std::chrono::high_resolution_clock::now();

	return true;
}

bool Player::Is_Time_Recv_Normal_Attack()
{
	if (false == time__recv_normal_attack.Is_cooltime())	return false;
	return time__recv_normal_attack.Reset_cooltime();
}

bool Player::Is_Time_Recv_SPELL()
{
	if (false == time__recv_spell.Is_cooltime())	return false;
	return time__recv_spell.Reset_cooltime();
}

bool Player::TEST_Move_packet()
{
	if (false == time__recv_move.Is_cooltime())	return false;
	if (false == time__recv_move.Reset_cooltime()) return false;

	target_x = (float)(rand() % (int)(PLAYER_VIEW_SIZE_WIDTH * 2.f)) - PLAYER_VIEW_SIZE_WIDTH + x;
	target_y = (float)(rand() % (int)(PLAYER_VIEW_SIZE_HEIGHT * 2.f)) - PLAYER_VIEW_SIZE_HEIGHT + y;

	return true;
}

bool Player::Is_Player()
{
	return true;
}

Type_POS Player::Get_Size() const
{
	return SIZE_PLAYER;
}

RESULT_PLAYER_MOVE Player::Move(std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>& vec_objects)
{
	Type_POS pred_x{ x }, pred_y{ y };
	RESULT_PLAYER_MOVE result;

	if (false == Update_position()) result = RPM_MODIFY;
	else							result = RPM_CORRECT;

	if (false == TrnMgr->Can_move(this)) {
		Reset_position(pred_x, pred_y);				
		return RPM_INCORRECT;
	}

	Get_Near_Object_in_near_set(vec_objects);

	SctMgr->Update_Sector(this, pred_x, pred_y);

	Update_Move(vec_objects);

	return result;
}

std::chrono::milliseconds Player::Get_respwan_time() const
{
	return TIME_PLAYER_RESPWAN;
}

Type_Damage Player::Normal_Attack(Object* const p_monster)
{
	Type_POS distance{ sqrtf(pow(x - p_monster->x, 2) + pow(y - p_monster->y, 2)) };
	
	if (DISTANCE_ALLOW_PLAYER_NORMAL_ATTACK + Get_Size() + p_monster->Get_Size() < distance)	return -1;

	Type_Damage damage{ str * 2 };

	if (true == p_monster->Hurt(damage, this))	return damage;

	return -1;
}

Type_Damage Player::Spell(Object* const p_monster)
{
	Type_POS distance{ sqrtf(pow(x - p_monster->x, 2) + pow(y - p_monster->y, 2)) };
	
	if (DISTANCE_ALLOW_PLAYER_SPELL < distance)	return -1;

	Type_Damage damage{ str * 2 };

	if (true == p_monster->Hurt(damage, this))	return damage;

	return -1;
}

void Player::Respwan()
{
	die = false;

	pred_move_time = std::chrono::high_resolution_clock::now();

	target_x = x;
	target_y = y;

	vel = 3.f;
	Hp = 100;
	str = 3;
}

SOCKET Player::Get_Socket() const
{
	return socket;
}

void Player::recv_move_time_ok()
{
	time__recv_move.Can_Do();
}

void Player::recv_normal_attack_time_ok()
{
	time__recv_normal_attack.Can_Do();
}

void Player::recv_spell_time_ok()
{
	time__recv_spell.Can_Do();
}

bool Player::Insert_Near_set_ifn_find(Object* const p_object)
{
	return near_set.insert_ifn_find(p_object);
}

void Player::Insert_Near_set(Object* const p_object)
{
	near_set.insert(p_object);
}
