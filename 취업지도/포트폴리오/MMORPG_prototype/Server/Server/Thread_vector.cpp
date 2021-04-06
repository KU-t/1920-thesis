#include "Thread_vector.h"

void Thread_vector::Add_Process_thread(int num)
{
	for (int i = 0; i < num; ++i)
		threads.emplace_back();
}
