#pragma once
#include "Monster_Base.h"

class Player;

class Monster_type3 : public Monster_Base
{
private:

public:
	Monster_type3() = delete;
	Monster_type3(const Type_ID&, const Type_POS&, const Type_POS&);

private:
	Object* Can_Attack_target_object();
	bool Can_Move();
	void Set_Move_Init_Target();

public:
	Object* Get_Nearest_Player();
	virtual void Awake__Set_Move_target() override final;
	virtual bool Is_far_from_init_position() override final;
	virtual RESULT_MONSTER_MOVE Move_or_Attack(std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>&, Object*&, Type_Damage&) override final;
	virtual Type_POS Get_Size() const override final;

	virtual MONSTER_TYPE Get_Type() const override final;
	virtual Type_Hp	Get_init_hp() const override;

	RESULT_MONSTER_MOVE Follow_Target(Object* const);
};


