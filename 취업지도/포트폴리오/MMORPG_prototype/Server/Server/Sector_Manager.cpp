#include "stdafx.h"
#include "Sector_Manager.h"
#include "Object_Manager.h"
#include "Player.h"
#include <cmath>
#include <algorithm>

// **************************
// ****** Sector_Base *******
// **************************

void Sector_Base::R_lock()
{
	mutex.lock_shared();
}

void Sector_Base::R_unlock()
{
	mutex.unlock_shared();
}

void Sector_Base::insert_without_mutex(Object* const p_object)
{
	set.insert(p_object);
}

void Sector_Base::insert(Object* const p_object)
{
	mutex.lock();

	set.insert(p_object);

	mutex.unlock();
}

void Sector_Base::erase(Object* const p_object)
{
	mutex.lock();

	for (Object* const p_set_object : set) {

		if (p_object == p_set_object) {
			set.erase(p_object);
			mutex.unlock();
			return;
		}
	}

	mutex.unlock();
}

// **************************
// ******* Sector_Manager *******
// **************************

Sector_Manager* Sector_Manager::SctMgr = nullptr;
Object_Manager* Sector_Manager::ObjMgr = nullptr;

Sector_Manager * Sector_Manager::Create_Sector_Manager()
{
	if (SctMgr == nullptr) {
		SctMgr = new Sector_Manager;
		printf("[Sector_Manager] - Create \n");
	}
	return SctMgr;
}

void Sector_Manager::Include_Manager(Object_Manager* _ObjMgr)
{
	if (ObjMgr == nullptr) {
		ObjMgr = _ObjMgr;
	}
}

void Sector_Manager::Init_Monsters_Sector()
{
	Object* p_object;
	Sector_Base* p_sector;

	for (int id = MAX_PLAYER; id < MAX_OBJECTS; ++id) {
		p_object = ObjMgr->Get_Object(id);
		p_sector = Get_Sector(p_object);
		p_sector->insert_without_mutex(p_object);
	}

	printf("[Sector_Manager] - Init Monsters \n");
}

Sector_Base& Sector_Manager::operator[](const int& index)
{
	return sector[index];
}

// public

bool Sector_Manager::is_in_left_edge(const int& x)
{
	return (x < WORLD_SECTOR_LEFT_EDGE);
}

bool Sector_Manager::is_in_right_edge(const int& x)
{
	return (WORLD_SECTOR_RIGHT_EDGE < x);
}

bool Sector_Manager::is_in_top_edge(const int& y)
{
	return (y < WORLD_SECTOR_TOP_EDGE);
}

bool Sector_Manager::is_in_bottom_edge(const int& y)
{
	return (WORLD_SECTOR_BOTTOM_EDGE < y);
}

bool Sector_Manager::is_x_min_sector(const int& x)
{
	return (x == 0);
}

bool Sector_Manager::is_x_max_sector(const int& x)
{
	return (x == WORLD_SECTOR_NUM_WIDTH - 1);
}

bool Sector_Manager::is_y_min_sector(const int& y)
{
	return (y == 0);
}

bool Sector_Manager::is_y_max_sector(const int& y)
{
	return (y == WORLD_SECTOR_NUM_HEIGHT - 1);
}

// public funtion

void Sector_Manager::insert(Object* const p_object)
{
	Sector_Base* sector{ Get_Sector(p_object) };
	sector->insert(p_object);
}

void Sector_Manager::erase(Object* const p_object)
{
	Sector_Base* sector{ Get_Sector(p_object) };
	sector->erase(p_object);
}

void Sector_Manager::Update_Sector(Object* const p_object, const Type_POS& pred_x, const Type_POS& pred_y)
{
	Sector_Base* pred_sector{ Get_Sector(pred_x, pred_y) };
	Sector_Base* curr_sector{ Get_Sector(p_object) };

	if (pred_sector == curr_sector)	return;

	curr_sector->insert(p_object);
	pred_sector->erase(p_object);
}


Sector_Base* Sector_Manager::Get_Sector(const int& x, const int& y)
{
	return &sector[x + (y * WORLD_SECTOR_NUM_WIDTH)];
}

Sector_Base * Sector_Manager::Get_Sector(const Type_POS& px, const Type_POS& py)
{
	int sector_x{ (int)px / WORLD_SECTOR_WIDTH_SIZE };
	int sector_y{ (int)py / WORLD_SECTOR_HEIGHT_SIZE };

	return Get_Sector(sector_x, sector_y);
}

Sector_Base* Sector_Manager::Get_Sector(const Object* const p_object)
{
	int sector_x{ (int)p_object->x / WORLD_SECTOR_WIDTH_SIZE };
	int sector_y{ (int)p_object->y / WORLD_SECTOR_HEIGHT_SIZE };
	
	return Get_Sector(sector_x, sector_y);
}


std::vector<Sector_Base*> Sector_Manager::Get_near_sector(const Type_POS& _x, const Type_POS& _y)
{
	int px = (int)_x, py = (int)_y;

	std::vector<Sector_Base*> vec_near_sector;

	int sector_x{ px / WORLD_SECTOR_WIDTH_SIZE };
	int sector_y{ py / WORLD_SECTOR_HEIGHT_SIZE };

	int client_x_in_sector{ px % WORLD_SECTOR_WIDTH_SIZE };
	int client_y_in_sector{ py % WORLD_SECTOR_HEIGHT_SIZE };

	vec_near_sector.emplace_back(Get_Sector(sector_x, sector_y));

	if (true == is_in_left_edge(client_x_in_sector)) {
		if (false == is_x_min_sector(sector_x)) {
			vec_near_sector.emplace_back(Get_Sector(sector_x - 1, sector_y));
			
			if (true == is_in_top_edge(client_y_in_sector)) {
				if (false == is_y_min_sector(sector_y)) {
					
					vec_near_sector.emplace_back(Get_Sector(sector_x, sector_y - 1));
					vec_near_sector.emplace_back(Get_Sector(sector_x - 1, sector_y - 1));
				}
			}

			else if (true == is_in_bottom_edge(client_y_in_sector)) {
				if (false == is_y_max_sector(sector_y)) {
					
					vec_near_sector.emplace_back(Get_Sector(sector_x, sector_y + 1));
					vec_near_sector.emplace_back(Get_Sector(sector_x - 1, sector_y + 1));
				}
			}
		}

		else {
			if (true == is_in_top_edge(client_y_in_sector)) {
				if (false == is_y_min_sector(sector_y)) 
					vec_near_sector.emplace_back(Get_Sector(sector_x, sector_y - 1));
			}

			else if (true == is_in_bottom_edge(client_y_in_sector)) {
				if (false == is_y_max_sector(sector_y))
					vec_near_sector.emplace_back(Get_Sector(sector_x, sector_y + 1));
			}
		}
	}
	
	else if (true == is_in_right_edge(client_x_in_sector)) {
		if (false == is_x_max_sector(sector_x)) {
			vec_near_sector.emplace_back(Get_Sector(sector_x + 1, sector_y));

			if (true == is_in_top_edge(client_y_in_sector)) {
				if (false == is_y_min_sector(sector_y)) {

					vec_near_sector.emplace_back(Get_Sector(sector_x, sector_y - 1));
					vec_near_sector.emplace_back(Get_Sector(sector_x + 1, sector_y - 1));
				}
			}

			else if (true == is_in_bottom_edge(client_y_in_sector)) {
				if (false == is_y_max_sector(sector_y)) {

					vec_near_sector.emplace_back(Get_Sector(sector_x, sector_y + 1));
					vec_near_sector.emplace_back(Get_Sector(sector_x + 1, sector_y + 1));
				}
			}
		}

		else {
			if (true == is_in_top_edge(client_y_in_sector)) {
				if (false == is_y_min_sector(sector_y))
					vec_near_sector.emplace_back(Get_Sector(sector_x, sector_y - 1));
			}

			else if (true == is_in_bottom_edge(client_y_in_sector)) {
				if (false == is_y_max_sector(sector_y))
					vec_near_sector.emplace_back(Get_Sector(sector_x, sector_y + 1));
			}
		}
	}

	else {
		if (true == is_in_top_edge(client_y_in_sector)) {
			if (false == is_y_min_sector(sector_y))
				vec_near_sector.emplace_back(Get_Sector(sector_x, sector_y - 1));
		}

		else if (true == is_in_bottom_edge(client_y_in_sector)) {
			if (false == is_y_max_sector(sector_y))
				vec_near_sector.emplace_back(Get_Sector(sector_x, sector_y + 1));
		}
	}


	return vec_near_sector;
}




