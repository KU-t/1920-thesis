#include "stdafx.h"
#include "CoolTime.h"

bool CoolTime::CAS(std::atomic_bool* memory, bool old_data, bool new_data)
{
	bool old_value = old_data;
	bool new_value = new_data;

	return std::atomic_compare_exchange_strong
	(reinterpret_cast<std::atomic_bool* >(memory), &old_value, new_value);
}

CoolTime::CoolTime()
{
	Do = true;
}

bool CoolTime::Is_cooltime()
{
	bool _Do { Do };
	
	return _Do;
}

bool CoolTime::Reset_cooltime()
{
	return CAS(&Do, true, false);;
}

void CoolTime::Can_Do()
{
	CAS(&Do, false, true);
}