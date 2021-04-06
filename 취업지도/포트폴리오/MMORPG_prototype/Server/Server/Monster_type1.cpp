#include "stdafx.h"
#include "Monster_type1.h"
#include "Terrain_Manager.h"
#include "Sector_Manager.h"
#include "Object_Manager.h"
#include "Timer_Manager.h"

Monster_type1::Monster_type1(const Type_ID& _id, const Type_POS& px, const Type_POS& py)
{
	id = _id;
	x = target_x = init_x = px;
	y = target_y = init_y = py;
	
	vel = 1.f;
	Hp = Get_init_hp();
	str = 5;
}

Object* Monster_type1::Can_Attack_target_object()
{
	Object* p_target_object{ target_object };

	if (nullptr == p_target_object)	return nullptr;

	if (nullptr == ObjMgr->Get_Object(p_target_object->id)) return nullptr;
	
	Type_POS distance{ sqrtf(pow(p_target_object->x - x, 2) + pow(p_target_object->y - y, 2)) };

	if (Get_Size() + p_target_object->Get_Size() < distance) return nullptr;
	
	return p_target_object;
}

bool Monster_type1::Can_Move()
{
	if (x != target_x || y != target_y) return true;

	if (true == init_moving) {
		init_moving = false;
		target_object = nullptr;
	}

	return false;
}

void Monster_type1::Set_Move_New_Target()
{
	target_object = nullptr;
	int rand_x{ (rand() % 10) - 5 };
	int rand_y{ (rand() % 10) - 5 };

	target_x += (float)rand_x;
	target_y += (float)rand_y;
}

void Monster_type1::Awake__Set_Move_target()
{
	int rand_x{ (rand() % 10) - 5 };
	int rand_y{ (rand() % 10) - 5 };

	target_x += (float)rand_x;
	target_y += (float)rand_y;

	pred_move_time = std::chrono::high_resolution_clock::now();
}

bool Monster_type1::Is_far_from_init_position()
{
	if (DISTANCE_MONSTER_TYPE1_FAR_FROM_INIT_X < x - init_x)		return true;
	if (DISTANCE_MONSTER_TYPE1_FAR_FROM_INIT_Y < init_x - x)		return true;
	if (DISTANCE_MONSTER_TYPE1_FAR_FROM_INIT_X < y - init_y)	return true;
	if (DISTANCE_MONSTER_TYPE1_FAR_FROM_INIT_Y < init_y - y)	return true;

	return false;
}

RESULT_MONSTER_MOVE Monster_type1::Move_or_Attack
(std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>& vec_objects, Object*& p_target_object, Type_Damage& damage)
{
	using namespace std;
	using namespace chrono;

	high_resolution_clock::time_point curr_time{ high_resolution_clock::now() };

	RESULT_MONSTER_MOVE result{ RMM_MOVE };

	p_target_object = Get_exist_target_object();

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

	else if (target_object != p_target_object) {
		Set_Move_New_Target();
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

	if( true == Is_Sleep__Update_Move(vec_objects))
		result = RMM_MOVE_TARGET;

	if (true == Is_far_from_init_position()) {
		Set_move_init_position();
		return RMM_MOVE_TARGET;
	}

	return	result;
}

Type_POS Monster_type1::Get_Size() const
{
	return SIZE_MONSTER_TYPE1;
}

bool Monster_type1::Hurt(Type_Damage damage, Object* const p_object)
{
	Object* old_target_object{ target_object };
	CAS_pointer(&target_object, old_target_object, p_object);

	return Object::Hurt(damage, nullptr);
}

MONSTER_TYPE Monster_type1::Get_Type() const
{
	return M_TYPE1;
}

Type_Hp Monster_type1::Get_init_hp() const
{
	return HP_MONSTER_TYPE1;
}

RESULT_MONSTER_MOVE Monster_type1::Follow_Target(Object* const p_target)
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
