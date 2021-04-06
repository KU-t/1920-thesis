#pragma once

#include <unordered_set>
#include <array>

class Player;
class Object;
class Object_Manager;
class Sector_Manager;
class Terrain_Manager;

class Sector_Base
{
private:
	std::unordered_set<Object*> set;
	std::shared_mutex mutex;

public:
	std::unordered_set<Object*>::iterator		begin() { return set.begin(); }
	std::unordered_set<Object*>::iterator		end() { return set.end(); }
	std::unordered_set<Object*>::const_iterator  begin() const { return set.begin(); }
	std::unordered_set<Object*>::const_iterator  end()	const { return set.end(); }

private:
	friend class Sector_Manager;
	void insert_without_mutex(Object* const);

public:
	void R_lock();
	void R_unlock();

	void insert(Object* const);
	void erase(Object* const);
};

class Sector_Manager
{
private:
	std::array<Sector_Base, NUMBER_OF_SECTOR> sector;

	static Sector_Manager* SctMgr;
	static Object_Manager* ObjMgr;
	Sector_Manager() = default;

public:
	static Sector_Manager* Create_Sector_Manager();
	void Include_Manager(Object_Manager*);
	void Init_Monsters_Sector();

public:
	Sector_Base& operator[](const int&);

private:
	bool is_in_left_edge(const int&);
	bool is_in_right_edge(const int&);
	bool is_in_top_edge(const int&);
	bool is_in_bottom_edge(const int&);
		 
	bool is_x_min_sector(const int&);
	bool is_x_max_sector(const int&);
	bool is_y_min_sector(const int&);
	bool is_y_max_sector(const int&);

	Sector_Base*	Get_Sector(const int&, const int&);
	Sector_Base*	Get_Sector(const Type_POS&, const Type_POS&);
	Sector_Base*	Get_Sector(const Object* const);

public:
	std::vector<Sector_Base*>	Get_near_sector(const Type_POS&, const Type_POS&);

	void insert(Object* const);
	void erase(Object* const);

	void Update_Sector(Object* const, const Type_POS&, const Type_POS&);
};