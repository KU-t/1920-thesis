#pragma once

#include <array>
#include <unordered_set>

class Object;
class Player;
class Monster_type1;
class Sector_Manager;
class Terrain_Manager;
class Timer_Manager;
class Server;

class Object_Manager
{
private:
	std::array<Object*, MAX_OBJECTS> objs;

public:
	Player* Login(const SOCKET&, const Type_ID&);
	void Logout(Player* const);
	void Set_Player_Move_OK(const Type_ID&);
	void Set_Player_Normal_Attack_OK(const Type_ID&);
	void Set_Player_Spell_OK(const Type_ID&);
	void Set_Monster_Attack_OK(const Type_ID&);


	Object*	Get_Object(const Type_ID&) const;
	
private:
	void insert(Object* const, const Type_ID&);
	void erase(Object* const);

private:
	static Object_Manager* ObjMgr;
	static Terrain_Manager* TrnMgr;

	Object_Manager() = default;

public:
	static Object_Manager* Create_Object_Manager();
	void Include_Manager(Sector_Manager* const, Terrain_Manager* const, Timer_Manager* const);
	void Create_Monsters();
};

