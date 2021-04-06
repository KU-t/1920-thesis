#pragma once

#include "Monster_Base.h"

class Monster_type1 : public Monster_Base
{
public:
	Monster_type1() = delete;
	Monster_type1(const Type_ID&, const Type_POS&, const Type_POS&);

private:
	Object* Can_Attack_target_object();
	bool Can_Move();
	void Set_Move_New_Target();

public:
	virtual void Awake__Set_Move_target() override final;
	virtual bool Is_far_from_init_position() override final;
	virtual RESULT_MONSTER_MOVE Move_or_Attack(std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>&, Object*&, Type_Damage&) override final;
	virtual Type_POS Get_Size() const override final;

	virtual bool Hurt(Type_Damage, Object*) override final;

	virtual MONSTER_TYPE Get_Type() const override final;
	virtual Type_Hp	Get_init_hp() const override;

	RESULT_MONSTER_MOVE Follow_Target(Object* const);
};

