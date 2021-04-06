#include "stdafx.h"
#include "Background.h"

Background::Background(const Type_ID& _id, const Type_ID& px, const Type_ID& py)
{
	id = _id;
	x = px;
	y = py;
}

void Background::Init()
{
	Add_sector();
}

void Background::Add_near_set(Object_Base * const)
{}

void Background::Insert_near_set(Client * const)
{}

void Background::Insert_near_set(Object_Base * const)
{}

void Background::Erase_near_set(Client * const)
{}

void Background::Erase_near_set(Object_Base * const)
{}

bool Background::Move(const char &, std::vector<std::pair<Object_Base*, RESULT_MOVE>>* const)
{
	return false;
}

bool Background::Hit(std::vector<Object_Base*>* const)
{
	return false;
}
