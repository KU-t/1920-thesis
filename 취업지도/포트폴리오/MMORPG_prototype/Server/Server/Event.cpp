#include "stdafx.h"
#include "Event.h"

EVENT::EVENT(const std::chrono::high_resolution_clock::time_point& _time, const EVENT_TYPE& _type, void* _detail)
{
	process_time = _time;
	type = _type;
	detail = _detail;
}

Detail_CoolTime_OK::Detail_CoolTime_OK(const Type_ID& _id)
{
	id = _id;
}

Detail_Monster_Move::Detail_Monster_Move(const Type_ID& _id)
{
	id = _id;
}

Detail_Respwan::Detail_Respwan(const Type_ID& _id)
{
	id = _id;
}
