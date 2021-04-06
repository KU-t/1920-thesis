#pragma once

#include "Monster_Base.h"

class Client;

class Background : public Object_Base
{
public:
	Background() = delete;
	Background(const Type_ID&, const Type_ID&, const Type_ID&);

public:

	virtual void Init() override final;

	virtual void Add_near_set(Object_Base* const) override final;

	virtual void Insert_near_set(Client* const) override final;
	virtual void Insert_near_set(Object_Base* const) override final;
	virtual void Erase_near_set(Client* const) override final;
	virtual void Erase_near_set(Object_Base* const) override final;

	virtual bool Move(const char&, std::vector<std::pair<Object_Base*, RESULT_MOVE>>* const) override final;
	virtual bool Hit(std::vector<Object_Base*>* const) override final;
};

