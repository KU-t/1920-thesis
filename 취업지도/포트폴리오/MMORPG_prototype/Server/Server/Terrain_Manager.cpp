#include "stdafx.h"
#include "Terrain_Manager.h"
#include "Object.h"
#include <fstream>

Terrain_Manager* Terrain_Manager::TrnMgr = nullptr;

Terrain_Manager::Terrain_Manager()
{
	height_map = new Type_POS[WORLD_MAP_SIZE];

	std::ifstream Terrain_File;
	Terrain_File.open("..\\..\\terrain_base.txt");

	if (!Terrain_File.is_open())
		printf("Can't open hight_map file.\n");

	printf("[Terrain_Manager] - Reading hight_map file...\n");
	for (int i = 0; i < WORLD_MAP_SIZE; i++)
		Terrain_File >> height_map[i];

	Terrain_File.close();
}

Type_POS Terrain_Manager::Get_terrain_height(const int& x, const int& y)
{
	return height_map[(int)(x) + ((int)(y) * (int)(WORLD_MAP_SIZE_WIDTH))];
}

Type_POS Terrain_Manager::Get_object_height(Object* const p_object)
{
	int xi{ (int)(p_object->x) }, yi{ (int)(p_object->y) };
	float xr{ p_object->x - (float)(xi) }, yr{ p_object->y - (float)(yi) };

	int next_xi{ xi + 1 }, next_yi{ yi + 1 };

	if (xi == WORLD_MAP_SIZE_WIDTH - 1)		next_xi = xi;
	if (yi == WORLD_MAP_SIZE_HEIGHT - 1)	next_yi = yi;

	Type_POS left_top_ter{ Get_terrain_height(xi, yi) };
	Type_POS right_top_ter{ Get_terrain_height(next_xi, yi) };
	Type_POS left_bottom_ter{ Get_terrain_height(xi, next_yi) };
	Type_POS right_bottom_ter{ Get_terrain_height(next_xi, next_yi) };

	Type_POS height;

#ifdef __LINEAR_INTERPOLATION_RECTANGLE__
	height = (left_top_ter * (1 - xr) * (1 - yr)) + (right_top_ter * xr * (1 - yr))
					+ (left_bottom_ter * (1 - xr) * yr) + (right_bottom_ter * xr * yr);
#endif

#ifdef __LINEAR_INTERPOLATION_TRIANGLE__
	float uh, vh;

	if (xr <= 1.f - yr) {
		uh = right_bottom_ter - left_bottom_ter;
		vh = left_bottom_ter - left_top_ter;
		height = left_bottom_ter + (uh * xr) + (vh * yr);
	}
	else {
		uh = right_top_ter - left_top_ter;
		vh = right_bottom_ter - right_top_ter;
		height = right_top_ter + (uh * (1.f - xr)) + (vh * (1.f - yr));
	}

#endif

	return height;
}

Type_POS Terrain_Manager::Get_height(const Type_POS& _x, const Type_POS& _y)
{
	int xi{ (int)(_x) }, yi{ (int)(_y) };
	float xr{ _x - (float)(xi) }, yr{ _y - (float)(yi) };

	int next_xi{ xi + 1 }, next_yi{ yi + 1 };

	if (xi == WORLD_MAP_SIZE_WIDTH - 1)		next_xi = xi;
	if (yi == WORLD_MAP_SIZE_HEIGHT - 1)	next_yi = yi;

	Type_POS	left_top_ter{ Get_terrain_height(xi, yi) },
		right_top_ter{ Get_terrain_height(next_xi, yi) },
		left_bottom_ter{ Get_terrain_height(xi, next_yi) },
		right_bottom_ter{ Get_terrain_height(next_xi, next_yi) };


	Type_POS height;

#ifdef __LINEAR_INTERPOLATION_RECTANGLE__
	height = (left_top_ter * (1 - xr) * (1 - yr)) + (right_top_ter * xr * (1 - yr))
		+ (left_bottom_ter * (1 - xr) * yr) + (right_bottom_ter * xr * yr);
#endif

#ifdef __LINEAR_INTERPOLATION_TRIANGLE__
	float uh, vh;

	if (xr <= 1.f - yr) {
		uh = right_bottom_ter - left_bottom_ter;
		vh = left_bottom_ter - left_top_ter;
		height = left_bottom_ter + (uh * xr) + (vh * yr);
	}
	else {
		uh = right_top_ter - left_top_ter;
		vh = right_bottom_ter - right_top_ter;
		height = right_top_ter + (uh * (1.f - xr)) + (vh * (1.f - yr));
	}
#endif

	return height;
}

Terrain_Manager * Terrain_Manager::Create_Terrain_Manager()
{
	if (TrnMgr == nullptr) {
		TrnMgr = new Terrain_Manager;
		printf("[Terrain_Manager] - Create \n");
	}
	return TrnMgr;
}

bool Terrain_Manager::Can_move(Object* const p_object)
{
	if (p_object->x < 0.f)								return false;
	if (WORLD_MAP_SIZE_WIDTH - 1.f < p_object->x)		return false;
	if (p_object->y < 0.f)								return false;
	if (WORLD_MAP_SIZE_HEIGHT - 1.f < p_object->y)		return false;

	//Type_POS height{ Get_object_height(p_object) };
	
	//if (height < 1.f)	return false;
	//if ( 4.f < height)	return false;

	return true;
}

bool Terrain_Manager::Can_Create_Object(const Type_POS& _x, const Type_POS& _y)
{
	//Type_POS height{ Get_height(_x, _y) };

	//if (height < 1.f)	return false;
	//if (4.f < height)	return false;

	return true;
}
