#include "stdafx.h"
#include "Object.h"
#include "Sector_Manager.h"
#include "Terrain_Manager.h"
#include "Timer_Manager.h"

Object_Manager* Object::ObjMgr = nullptr;
Sector_Manager* Object::SctMgr = nullptr;
Terrain_Manager* Object::TrnMgr = nullptr;
Timer_Manager* Object::TmrMgr = nullptr;

// protected

void Object::Reset_position(const Type_POS& pred_x, const Type_POS& pred_y)
{
	x = pred_x;
	y = pred_y;

	target_x = pred_x;
	target_y = pred_y;
}

void Object::Include_Object_Manager(Object_Manager* const _ObjMgr)
{
	if (ObjMgr == nullptr)	ObjMgr = _ObjMgr;
}

void Object::Include_Sector_Manager(Sector_Manager* const _SctMgr)
{
	if (SctMgr == nullptr) 	SctMgr = _SctMgr;
}

void Object::Include_Terrain_Manager(Terrain_Manager* const _TrnMgr)
{
	if (TrnMgr == nullptr) 	TrnMgr = _TrnMgr;
}

void Object::Include_Timer_Manager(Timer_Manager* const _TmrMgr)
{
	if (TmrMgr == nullptr)
		TmrMgr = _TmrMgr;
}


// public function

Type_ID Object::Get_ID() const
{
	return id;
}

bool Object::CAS_Hp(Type_atomic_Hp* memory, Type_Hp old_data, Type_Hp new_data)
{
	Type_Hp old_value = old_data;
	Type_Hp new_value = new_data;

	return std::atomic_compare_exchange_strong
	(reinterpret_cast<Type_atomic_Hp*>(memory), &old_value, new_value);
}

bool Object::CAS_bool(std::atomic_bool* memory, bool old_data, bool new_data)
{
	bool old_value = old_data;
	bool new_value = new_data;

	return std::atomic_compare_exchange_strong
	(reinterpret_cast<std::atomic_bool*>(memory), &old_value, new_value);
}

bool Object::CAS_pointer(Object** memory, Object* old_addr, Object* new_addr)
{
	POINTER_SIZE p_old_addr{ reinterpret_cast<POINTER_SIZE>(old_addr) };
	POINTER_SIZE p_new_addr{ reinterpret_cast<POINTER_SIZE>(new_addr) };

	return std::atomic_compare_exchange_strong
	(reinterpret_cast<POINTER_TYPE*>(memory), &p_old_addr, p_new_addr);
}

bool Object::Hurt(Type_Damage damage, Object*)
{
	bool is_die;
	Type_Hp current_hp;
	Type_Hp new_hp;

	while (true) {
		is_die = die;
		if (true == is_die)	return false;

		current_hp = Hp;
		if (current_hp < 1)	return false;

		new_hp = current_hp - damage;
		if (new_hp < 0)	new_hp = 0;

		if (true == CAS_Hp(&Hp, current_hp, new_hp)) {
			if (0 == new_hp) {
				die = true;
				//TmrMgr->Push_Event_Respwan(this);
			}
			return true;
		}
	}
}

bool Object::Is_exist() const
{
	return exist;
}

bool Object::Is_die() const
{
	return die;
}
