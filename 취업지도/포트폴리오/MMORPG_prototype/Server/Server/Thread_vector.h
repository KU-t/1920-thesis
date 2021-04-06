#pragma once

#include <vector>
#include <thread>

class Thread_vector
{
private:
	std::vector<std::thread> threads;

public:
	void Add_Process_thread(int);
};

