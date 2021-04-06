#include "stdafx.h"
#include <iostream>

#include "Object_Manager.h"
#include "Terrain_Manager.h"
#include "Object.h"
#include "Player.h"
#include "Monster_type1.h"
#include "Monster_type2.h"
#include "Monster_type3.h"

Object_Manager* Object_Manager::ObjMgr = nullptr;
Terrain_Manager* Object_Manager::TrnMgr = nullptr;

Object_Manager* Object_Manager::Create_Object_Manager()
{
	if (ObjMgr == nullptr) {
		ObjMgr = new Object_Manager();
		printf("[Object_Manager] - Create \n");
	}
	return ObjMgr;
}


Player* Object_Manager::Login(const SOCKET& socket_player, const Type_ID& index)
{
	Player* p_client{ new Player(socket_player, index) };
	
	objs[index] = reinterpret_cast<Object*>(p_client);
	
	return p_client;
}

void Object_Manager::Logout(Player* const p_player)
{
	objs[p_player->id] = nullptr;
	p_player->Logout();
}

void Object_Manager::Set_Player_Move_OK(const Type_ID& id)
{
	Object* p_object{ objs[id] };
	if (p_object == nullptr)	return;
	
	Player* p_client{ reinterpret_cast<Player*>(p_object) };

	p_client->recv_move_time_ok();
}

void Object_Manager::Set_Player_Normal_Attack_OK(const Type_ID& id)
{
	Object* p_object{ objs[id] };
	if (p_object == nullptr)	return;

	Player* p_client{ reinterpret_cast<Player*>(p_object) };

	p_client->recv_normal_attack_time_ok();
}

void Object_Manager::Set_Player_Spell_OK(const Type_ID& id)
{
	Object* p_object{ objs[id] };
	if (p_object == nullptr)	return;

	Player* p_client{ reinterpret_cast<Player*>(p_object) };

	p_client->recv_spell_time_ok();
}

void Object_Manager::Set_Monster_Attack_OK(const Type_ID& id)
{
	Object* p_object{ objs[id] };
	if (p_object == nullptr)	return;

	Monster_Base* p_monster{ reinterpret_cast<Monster_Base*>(p_object) };

	p_monster->attack_ok();
}

void Object_Manager::insert(Object* const p_object, const Type_ID& index)
{
	objs[index] = p_object;
}

void Object_Manager::erase(Object* const p_object)
{
	objs[p_object->id] = nullptr;
}

void Object_Manager::Include_Manager
	(Sector_Manager* const _SctMgr, Terrain_Manager* const _TrnMgr, Timer_Manager* const _TmrMgr)
{
	if (TrnMgr == nullptr) TrnMgr = _TrnMgr;

	Object::Include_Object_Manager(this);
	Object::Include_Sector_Manager(_SctMgr);
	Object::Include_Terrain_Manager(_TrnMgr);
	Object::Include_Timer_Manager(_TmrMgr);
}

void Object_Manager::Create_Monsters()
{
	/*for (int id = MAX_PLAYER; id < MAX_MONSTER_TYPE1; ++id ) 
		objs[id] = new Monster_type1(id, 455.f, 204.f);
	
	for (int id = MAX_MONSTER_TYPE1; id < MAX_MONSTER_TYPE2; ++id)
		objs[id] = new Monster_type2(id, 455.f, 209.f);

	for (int id = MAX_MONSTER_TYPE2; id < MAX_MONSTER_TYPE3; ++id)
		objs[id] = new Monster_type3(id, 459.f, 209.f);*/

	
	Type_POS px = 0.f, py = 0.f;
	for (int id = MAX_PLAYER; id < MAX_MONSTER_TYPE1; ) {
		if (true == TrnMgr->Can_Create_Object(px, py)) {
			objs[id] = new Monster_type1(id, px, py);
			++id;
		}

		px = (Type_POS)(px + 7.f);
		if (1000.f <= px) {
			px = 0.f;
			py = (py + 7.f);
		}
	}

	px = 0.f, py = 0.f;

	for (int id = MAX_MONSTER_TYPE1; id < MAX_MONSTER_TYPE2; ) {
		if (true == TrnMgr->Can_Create_Object(px, py)) {
			objs[id] = new Monster_type2(id, px, py);
			++id;
		}

		px = (Type_POS)(px + 10.f);
		if (1000.f <= px) {
			px = 0.f;
			py = (py + 10.f);
		}
	}
	
	px = 0.f, py = 0.f;

	for (int id = MAX_MONSTER_TYPE2; id < MAX_MONSTER_TYPE3; ) {
		if (true == TrnMgr->Can_Create_Object(px, py)) {
			objs[id] = new Monster_type3(id, px, py);
			++id;
		}

		px = (Type_POS)(px + 15.f);
		if (1000.f <= px) {
			px = 0.f;
			py = (py + 15.f);
		}
	}

	printf("[Object_Manager] - Create Monsters \n");
}

Object* Object_Manager::Get_Object(const Type_ID& id) const
{
	Object* p_object{ objs[id] };

	if (nullptr == p_object)	return nullptr;
	if (false == p_object->Is_exist())	return nullptr;

	return p_object;
}
