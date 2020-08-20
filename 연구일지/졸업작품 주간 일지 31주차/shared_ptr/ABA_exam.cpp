#include <iostream>
#include <vector>
#include <thread>

#include "linux_lockfree_shared_ptr.h"

LFSP::shared_ptr<int> a,b;

void thread1()
{
	int i = 0;
	while (true)
	{
		a = LFSP::make_shared<int>(i++);
	}
}

void thread2()
{
	while (true)
	{
		b = a;
		std::cout << *b << std::endl;
	}
}

int main{

	std::vector<std::thread> threads;

	threads.emplace_back(hpsp_thread_func, num_thread);

	for (auto &th : threads)	th.join();

}