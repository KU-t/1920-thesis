#include <iostream>
#include <vector>
#include <random>

#include "HP_lockfree_shared_ptr.h"

#define Num_shared_ptr			10
#define Num_weak_per_shared		1000
#define Num_test				80000000
#define Num_of_threads			8

using namespace std::chrono;

class Node {
public:
	int  val;

	Node() {}
	Node(int i) {
		val = i;
	}
};

HPLFSP::shared_ptr<Node>	shared[Num_shared_ptr];
HPLFSP::weak_ptr<Node>	weak[Num_shared_ptr * Num_weak_per_shared];
int Num_of_weak_of_shared[Num_shared_ptr];

void sp_thread_func(int num_thread) {
	int index1, index2;
	HPLFSP::weak_ptr<Node> empty;

	for (int i = 0; i < Num_test / num_thread; i++) {
		index1 = rand() % (Num_shared_ptr * Num_weak_per_shared);
		index2 = rand() % (Num_shared_ptr * Num_weak_per_shared);
		
		empty = weak[index1];
		weak[index1] = weak[index2];
		weak[index2] = empty;
	}
	HPLFSP::shared_ptr<Node> s = weak[index1].lock();
}

void main() {

	// Init
	HPLFSP::HP_helper* helper = new HPLFSP::HP_helper();
	for (int i = 0; i < Num_shared_ptr; ++i)
		shared[i] = HPLFSP::make_shared<Node>(helper, Node(i));


	// test information

	std::cout << "\t\t\t------------------------------------" << std::endl;
	std::cout << "\t\t\t  Num of test : " << Num_test << " (average time)\n\n";

	std::cout << "\t\t\t\tNum shared_ptr = " << Num_shared_ptr << "\n";
	std::cout << "\t\t\t\tNum weak_ptr = " << Num_shared_ptr << "*" << Num_weak_per_shared << "\n";
	std::cout << "\t\t\t------------------------------------" << std::endl;

	for (int num_thread = Num_of_threads; num_thread <= Num_of_threads; num_thread *= 2) {

		// init
		{
			for (int i = 0; i < Num_shared_ptr; ++i)
				for (int j = 0; j < Num_weak_per_shared; ++j)
					weak[i * Num_weak_per_shared + j] = shared[i];

			for (int i = 0; i < Num_shared_ptr; ++i) 
				Num_of_weak_of_shared[i] = 0;
		}

		std::vector<std::thread> threads;

		auto start_time = high_resolution_clock::now();

		for (int i = 0; i < num_thread; ++i)
			threads.emplace_back(sp_thread_func, num_thread);

		for (auto &th : threads)	th.join();

		auto end_time = high_resolution_clock::now();
		auto exec_time = end_time - start_time;
		long long exec_ms = duration_cast<milliseconds>(exec_time).count();

		for (int i = 0; i < Num_shared_ptr * Num_weak_per_shared; ++i)
			Num_of_weak_of_shared[weak[i].get_ptr()->val]++;

		{	// show table
			std::cout << "Thread : " << num_thread;

			for (int i = 0; i < Num_shared_ptr; ++i)
				std::cout << "\ts" << i;
		}

		{	// info shared_ptr
			std::cout << "\nshared count";
			int sum_s = 0;

			for (int i = 0; i < Num_shared_ptr; ++i) {
				std::cout << "\t" << shared[i].weak_count() - 1;
				sum_s += shared[i].weak_count() - 1;
			}
			std::cout << "\tSum : " << sum_s;
		}
		

		{	// info weak_ptr
			std::cout << "\nweak count";
			int sum_w = 0;

			for (int i = 0; i < Num_shared_ptr; ++i) {
				std::cout << "\t" << Num_of_weak_of_shared[i];
				sum_w += Num_of_weak_of_shared[i];
			}
			std::cout << "\tSum : " << sum_w;
		}
		
		{
			std::cout << "\nsame check";
			for (int i = 0; i < Num_shared_ptr; ++i) {
				std::cout << "\t" << std::boolalpha <<(shared[i].weak_count() - 1 == Num_of_weak_of_shared[i]);
			}
		}

		std::cout << "\n Exec_Time : " << exec_ms << "ms\n\n";
	}
}