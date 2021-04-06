#pragma once

#include <array>

#include "Near_set.h"
#include "Cooltime.h"

class Object_Manager;
class Sector_Manager;
class Terrain_Manager;
class Timer_Manager;
class Send_Manager;
class Player;
class Monster_Base;

struct Expand_Overlapped
{
	WSAOVERLAPPED	over;
	EXOVER_TYPE		type;
};

struct Player_Expand_Overlapped
{
	Expand_Overlapped exover;
	WSABUF			wsabuf;
	char			IObuf[MAX_BUFFER];
};

class Object_base
{
public:
	virtual bool Is_Player() = 0;
	virtual bool is_near(Object* const) = 0;
	virtual bool Hurt(Type_Damage, Object*) = 0;
	virtual Type_POS Get_Size() const = 0;
	virtual std::chrono::milliseconds Get_respwan_time() const = 0;
};

class Object : public Object_base
{
protected:
	Type_ID		id;
	bool exist;
	bool die;

	std::chrono::high_resolution_clock::time_point pred_move_time;
	Type_POS	target_x, target_y;
	
	Type_POS	x, y;
	Type_POS	vel;
	Type_atomic_Hp Hp;
	Type_Damage	str;

	static Object_Manager* ObjMgr;
	static Sector_Manager* SctMgr;
	static Terrain_Manager* TrnMgr;
	static Timer_Manager* TmrMgr;

	friend class Object_Manager;
	friend class Sector_Manager;
	friend class Terrain_Manager;
	friend class Timer_Manager;
	friend class Send_Manager;
	friend class Player;
	friend class Monster_Base;
	friend class Monster_type1;
	friend class Monster_type2;
	friend class Monster_type3;

	void Reset_position(const Type_POS&, const Type_POS&);

public:
	Object() = default;
	virtual ~Object()	= default;

	static void Include_Object_Manager(Object_Manager* const);
	static void Include_Sector_Manager(Sector_Manager* const);
	static void Include_Terrain_Manager(Terrain_Manager* const);
	static void Include_Timer_Manager(Timer_Manager* const);

protected:
	bool CAS_Hp(Type_atomic_Hp*, Type_Hp, Type_Hp);
	bool CAS_bool(std::atomic_bool*, bool, bool);
	bool CAS_pointer(Object**, Object*, Object*);

public:	
	Type_ID		Get_ID()		const;

	virtual bool Is_Player() override { return false; };
	virtual bool is_near(Object* const) override { return false; };
	virtual bool Hurt(Type_Damage, Object*) override;
	virtual Type_POS Get_Size() const override { return 0.f; };
	virtual std::chrono::milliseconds Get_respwan_time() const override { return std::chrono::milliseconds(0); };
	bool Is_exist() const;
	bool Is_die() const;
};



