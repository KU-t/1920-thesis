#pragma once

class Object;

class Terrain_Manager
{
private:
	Type_POS* height_map;

	static Terrain_Manager* TrnMgr;

	Terrain_Manager();

private:
	Type_POS Get_terrain_height(const int&, const int&);
	Type_POS Get_object_height(Object* const);
	Type_POS Get_height(const Type_POS&, const Type_POS&);

public:
	static Terrain_Manager* Create_Terrain_Manager();

	bool Can_move(Object* const);
	bool Can_Create_Object(const Type_POS&, const Type_POS&);
};

