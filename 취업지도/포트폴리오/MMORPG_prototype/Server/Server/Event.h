#pragma once

class Player;

struct EVENT
{
	std::chrono::high_resolution_clock::time_point process_time;
	EVENT_TYPE type;
	void* detail;

	bool operator<(const EVENT& other) const {
		return process_time > other.process_time;
	}

	EVENT() = default;
	EVENT(const std::chrono::high_resolution_clock::time_point&, const EVENT_TYPE&, void*);
};

struct Detail_CoolTime_OK
{
	Type_ID id;
	Detail_CoolTime_OK() = delete;
	Detail_CoolTime_OK(const Type_ID&);
};

struct Detail_Monster_Move
{
	Type_ID id;
	Detail_Monster_Move() = delete;
	Detail_Monster_Move(const Type_ID&);
};

struct Detail_Respwan
{
	Type_ID id;
	Detail_Respwan() = delete;
	Detail_Respwan(const Type_ID&);
};
