#pragma once

#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

#include <thread>
#include <atomic>
#include <chrono>
#include <list>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <cmath>
#include <algorithm>

#include "..\..\protocol.h"

using Type_sector = int;

#ifdef _WIN64
using POINTER_SIZE = long long;
using POINTER_TYPE = std::atomic_llong;
#else
using POINTER_SIZE = long;
using POINTER_TYPE = std::atomic_long;
#endif

using Type_atomic_Hp = std::atomic_int;

enum EXOVER_TYPE { EX_ACCEPT, EX_RECEIVE, EX_SEND,
	EX_MONSTER_AWAKE, EX_MONSTER_MOVE, EX_MONSTER_MOVE_START, EX_RESPWAN_PLAYER, EX_RESPWAN_MONSTER};

enum EVENT_TYPE	 { EV_PLAYER_MOVE_OK, EV_PLAYER_Normal_Attack_OK, EV_PLAYER_Spell_OK,
		EV_MONSTER_MOVE, EV_MONSTER_MOVE_START, EV_MONSTER_ATTACK, EV_RESPWAN};

enum RESULT_PLAYER_MOVE {RPM_CORRECT, RPM_MODIFY, RPM_INCORRECT};
enum RESULT_MONSTER_MOVE { RMM_MOVE, RMM_CANT_MOVE, RMM_MOVE_TARGET, RMM_ATTACK, RMM_MOVE_INIT, RMM_COLLISION };


enum RESULT_MOVE_OBJECT_TYPE { RMOT_UNEXIST,
	RMOT_PLAYER_SIGHT_IN, RMOT_PLAYER_SIGHT_OUT, RMOT_PLAYER_NEAR,
	RMOT_OBJECT_SIGHT_IN, RMOT_OBJECT_SIGHT_OUT, RMOT_OBJECT_NEAR };


constexpr int NUMBER_OF_THREAD__SERVER_PROCESS{ 3 };
constexpr int NUMBER_OF_THREAD__TIMER_PROCESS{ 2 };

// buffer info
constexpr int MAX_BUFFER{ 4096 };
constexpr int MIN_BUFFER{ 1024 };

constexpr int WORLD_SECTOR_WIDTH_SIZE{ (int)(PLAYER_VIEW_SIZE_WIDTH * 4.f) };
constexpr int WORLD_SECTOR_HEIGHT_SIZE{ (int)(PLAYER_VIEW_SIZE_HEIGHT * 4.f) };

constexpr int WORLD_SECTOR_NUM_WIDTH{ (int)(WORLD_MAP_SIZE_WIDTH / WORLD_SECTOR_WIDTH_SIZE) };
constexpr int WORLD_SECTOR_NUM_HEIGHT{ (int)(WORLD_MAP_SIZE_HEIGHT / WORLD_SECTOR_HEIGHT_SIZE) };

constexpr int NUMBER_OF_SECTOR{ WORLD_SECTOR_NUM_WIDTH * WORLD_SECTOR_NUM_HEIGHT };

constexpr Type_POS WORLD_SECTOR_LEFT_EDGE{ PLAYER_VIEW_SIZE_WIDTH };
constexpr Type_POS WORLD_SECTOR_RIGHT_EDGE{ WORLD_SECTOR_WIDTH_SIZE - PLAYER_VIEW_SIZE_WIDTH };
constexpr Type_POS WORLD_SECTOR_TOP_EDGE{ PLAYER_VIEW_SIZE_HEIGHT };
constexpr Type_POS WORLD_SECTOR_BOTTOM_EDGE	{ WORLD_SECTOR_HEIGHT_SIZE - PLAYER_VIEW_SIZE_HEIGHT};

constexpr Type_POS MONSTER_PLAYER_NEAR_WIDTH{ PLAYER_VIEW_SIZE_WIDTH + 1.5f };
constexpr Type_POS MONSTER_PLAYER_NEAR_HEIGHT{ PLAYER_VIEW_SIZE_HEIGHT + 0.75f };

constexpr auto TIME_MOVE_INTERPOLATION{ std::chrono::duration<float, std::milli>(std::chrono::milliseconds(CS_TIME_SEND_PLAYER_MOVE * 2)).count() };

constexpr int TIME_RECV_ALLOW_LATENCY{ 200 };
constexpr int TIME_RECV_PLAYER_MOVE{ CS_TIME_SEND_PLAYER_MOVE - TIME_RECV_ALLOW_LATENCY };// milliseconds
constexpr int TIME_RECV_PLAYER_NORMAL_ATTACK{ CS_TIME_SEND_PLAYER_MOVE - TIME_RECV_ALLOW_LATENCY };// milliseconds
constexpr int TIME_RECV_PLAYER_SPELL{ CS_TIME_SEND_PLAYER_MOVE - TIME_RECV_ALLOW_LATENCY };// milliseconds

constexpr int TIME_MONSTER_TYPE1_NEXT_MOVE{ 2000 };	// milliseconds
constexpr int TIME_MONSTER_TYPE2_NEXT_MOVE{ S_TIME_UPDATE_OBJECT_MOVE };	// milliseconds

constexpr int TIME_MONSTER_TYPE1_ATTACK{ 700 };	// milliseconds
constexpr int TIME_MONSTER_TYPE2_ATTACK{ 1500 };	// milliseconds
constexpr int TIME_MONSTER_TYPE3_ATTACK{ 1000 };	// milliseconds

constexpr std::chrono::milliseconds TIME_PLAYER_RESPWAN{ 5000 };	// milliseconds
constexpr std::chrono::milliseconds TIME_MONSTER_RESPWAN{ 10000 };	// milliseconds

constexpr Type_POS DISTANCE_ALLOW_PLAYER_NORMAL_ATTACK{ DISTANCE_PLAYER_NORMAL_ATTACK + 0.3f };
constexpr Type_POS DISTANCE_ALLOW_PLAYER_SPELL{ DISTANCE_PLAYER_SPELL + 0.5f };

constexpr Type_POS DISTANCE_MONSTER_TYPE1_FAR_FROM_INIT_X{ OBJECT_NEAR_WIDTH / 2 };
constexpr Type_POS DISTANCE_MONSTER_TYPE1_FAR_FROM_INIT_Y{ OBJECT_NEAR_HEIGHT / 2 };
constexpr Type_POS DISTANCE_MONSTER_TYPE2_FAR_FROM_INIT_X{ OBJECT_NEAR_WIDTH };
constexpr Type_POS DISTANCE_MONSTER_TYPE2_FAR_FROM_INIT_Y{ OBJECT_NEAR_HEIGHT };
constexpr Type_POS DISTANCE_MONSTER_TYPE3_FAR_FROM_INIT_X{ OBJECT_NEAR_WIDTH * 2 };
constexpr Type_POS DISTANCE_MONSTER_TYPE3_FAR_FROM_INIT_Y{ OBJECT_NEAR_HEIGHT * 2 };