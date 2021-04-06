#include "stdafx.h"
#include "Monster_type2.h"
#include "Terrain_Manager.h"
#include "Sector_Manager.h"
#include "Timer_Manager.h"
#include "Object_Manager.h"

Monster_type2::Monster_type2(const Type_ID& _id, const Type_POS& px, const Type_POS& py)
{
	id = _id;
	x = target_x = init_x = px;
	y = target_y = init_y = py;

	vel = 1.f;
	Hp = Get_init_hp();
	str = 10;
}

Object* Monster_type2::Can_Attack_target_object()
{
	if (nullptr == target_object)	return nullptr;

	Object* p_target_object{ ObjMgr->Get_Object(target_object->id) };

	if (nullptr == p_target_object) return nullptr;

	Type_POS distance{ sqrtf(pow(p_target_object->x - x, 2) + pow(p_target_object->y - y, 2)) };

	if (Get_Size() + p_target_object->Get_Size() < distance)
		return nullptr;

	return p_target_object;
}

bool Monster_type2::Can_Move()
{
	if (x != target_x || y != target_y) return true;
	
	if (true == init_moving) {
		init_moving = false;
		target_object = nullptr;
	}

	return false;
}

void Monster_type2::Set_Move_Init_Target()
{
	target_x = init_x;
	target_y = init_y;
}

Object* Monster_type2::Get_Nearest_Player()
{
	std::vector<Sector_Base*> vec_near_sector{ SctMgr->Get_near_sector(x, y) };
	Object* nearest_player{ nullptr };
	Type_POS nearest_len{ 100.f };

	for (Sector_Base* const sector : vec_near_sector) {

		sector->R_lock();

		for (Object* const p_sector_object : *sector) {

			if (false == p_sector_object->Is_exist()) continue;

			if (false == p_sector_object->Is_Player()) continue;

			if (false == is_near(p_sector_object))	continue;

			if (true == p_sector_object->Is_die())	continue;

			Type_POS len_player{ abs(p_sector_object->x - x) + abs(p_sector_object->y - y) };

			if (len_player < nearest_len) {
				nearest_len = len_player;
				nearest_player = p_sector_object;
			}

		}

		sector->R_unlock();
	}

	return nearest_player;
}

void Monster_type2::Awake__Set_Move_target()
{
	pred_move_time = std::chrono::high_resolution_clock::now();
}

bool Monster_type2::Is_far_from_init_position()
{
	if (DISTANCE_MONSTER_TYPE2_FAR_FROM_INIT_X < x - init_x)		return true;
	if (DISTANCE_MONSTER_TYPE2_FAR_FROM_INIT_Y < init_x - x)		return true;
	if (DISTANCE_MONSTER_TYPE2_FAR_FROM_INIT_X < y - init_y)	return true;
	if (DISTANCE_MONSTER_TYPE2_FAR_FROM_INIT_Y < init_y - y)	return true;

	return false;
}

RESULT_MONSTER_MOVE Monster_type2::Move_or_Attack
	(std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>& vec_objects, Object*& p_target_object, Type_Damage& damage)
{
	using namespace std;
	using namespace chrono;

	high_resolution_clock::time_point curr_time{ high_resolution_clock::now() };

	RESULT_MONSTER_MOVE result{ RMM_MOVE };

	target_object = Get_Nearest_Player();
	p_target_object = target_object;

	if (nullptr != p_target_object) {
		result = Follow_Target(p_target_object);

		if (RMM_ATTACK == result) {
			damage = str;
			Get_Near_Player(vec_objects);
			Set_Update_time(curr_time);
			return RMM_ATTACK;
		}

		if (RMM_COLLISION == result) {
			Set_Update_time(curr_time);
			return RMM_COLLISION;
		}
	}

	else {
		Set_Move_Init_Target();
		result = RMM_MOVE_TARGET;
	}

	if (false == Can_Move()) {
		Set_Update_time(curr_time);
		return RMM_CANT_MOVE;
	}

	Type_POS pred_x{ x }, pred_y{ y };

	Update_position(curr_time);

	if (false == TrnMgr->Can_move(this)) {
		Reset_position(pred_x, pred_y);
		return RMM_CANT_MOVE;
	}

	Get_Near_Player(vec_objects);

	SctMgr->Update_Sector(this, pred_x, pred_y);

	if (true == Is_Sleep__Update_Move(vec_objects))
		result = RMM_MOVE_TARGET;

	if (true == Is_far_from_init_position()) {
		Set_move_init_position();
		return RMM_MOVE_TARGET;
	}
	
	return	result;
}

Type_POS Monster_type2::Get_Size() const
{
	return SIZE_MONSTER_TYPE2;
}

MONSTER_TYPE Monster_type2::Get_Type() const
{
	return M_TYPE2;
}

Type_Hp Monster_type2::Get_init_hp() const
{
	return HP_MONSTER_TYPE2;
}

RESULT_MONSTER_MOVE Monster_type2::Follow_Target(Object* const p_target)
{
	if (true == init_moving)	return RMM_MOVE;

	Type_POS distance{ sqrtf(pow(p_target->x - x, 2) + pow(p_target->y - y, 2)) };

	if (Get_Size() + p_target->Get_Size() < distance) {
		target_x = p_target->target_x;
		target_y = p_target->target_y;
		return RMM_MOVE_TARGET;
	}

	target_x = x;
	target_y = y;
	
	if (false == time__attack.Reset_cooltime()) return RMM_COLLISION;

	TmrMgr->Push_Event_Monster_type_Attack_OK(this);

	if (false == p_target->Hurt(str, this)) return RMM_MOVE_TARGET;

	return RMM_ATTACK;
}
