#pragma once

#include "Object.h"
#include "Near_set.h"

enum MONSTER_TYPE {M_TYPE1, M_TYPE2, M_TYPE3};

class Monster_Base : public Object
{
public:
	Expand_Overlapped move_exover;

protected:
	std::atomic_bool sleep;
	
	Type_POS init_x, init_y;
	Object* target_object;
	CoolTime time__attack;
	bool init_moving;

protected:
	void Get_Near_Player(std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>&);
	bool Is_Sleep__Update_Move(std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>&);
	virtual bool is_near(Object* const) override final;
	virtual Type_POS Get_Size() const override { return 0.f; };
	void Set_Update_time(std::chrono::high_resolution_clock::time_point);
	void Update_position(std::chrono::high_resolution_clock::time_point);
	Object* Get_exist_target_object();

public:
	Monster_Base();

	bool Awake();
	bool Sleep();
	bool Is_sleep();

	bool Can_Hurt();

	void attack_ok();
	virtual bool Is_Player() override final;
	
	virtual bool Is_far_from_init_position() = 0;
	virtual void Awake__Set_Move_target() = 0;

	void Set_move_init_position();
	virtual RESULT_MONSTER_MOVE Move_or_Attack(std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>&, Object*&, Type_Damage&) = 0;
	virtual std::chrono::milliseconds Get_respwan_time() const override final;

	virtual MONSTER_TYPE Get_Type() const = 0;
	virtual Type_Hp	Get_init_hp() const = 0;
	void Respwan();
};

