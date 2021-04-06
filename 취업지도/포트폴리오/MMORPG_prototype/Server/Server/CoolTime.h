#pragma once

class CoolTime
{
private:
	std::atomic_bool Do;

private:
	bool CAS(std::atomic_bool*, bool, bool);

public:
	CoolTime();

	bool Is_cooltime();
	bool Reset_cooltime();
	void Can_Do();
};

