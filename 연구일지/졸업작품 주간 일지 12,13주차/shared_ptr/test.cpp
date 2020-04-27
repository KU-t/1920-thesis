#include <iostream>
#include <thread>
#include <vector>
#include "blocking_shared_ptr.h"

using namespace LFSP;

class TEST : public enable_shared_from_this<TEST>
{
public:
	void info() //***********test
	{
		std::cout << "ID : " << ID << " / HP : " << HP << std::endl;
	}

	TEST()
		: ID(0), HP(0)
	{}

	TEST(int _ID, int _HP)
		: ID(_ID), HP(_HP)
	{}

	shared_ptr<TEST> get()
	{
		//return shared_ptr<TEST>(this);
		return shared_from_this();
	}

private:
	int ID;
	int HP;
};

void blocking_test()
{
	{
		printf("**********************atomic_load(shared_ptr)***********************\n");
		shared_ptr<int> intptr0(new int(10));
		
		int int0 = atomic_load(intptr0);
		std::cout << int0 << std::endl;

		/*shared_ptr<int> intptr1 = atomic_load(intptr0);
		std::cout << *intptr1 << std::endl;*/
	}

	{	
		/*
		printf("**********************atomic_load(weak_ptr)***********************\n");
		shared_ptr<int> intptr0(new int(100));
		weak_ptr<int> intptr1(intptr0);

		int int1 = atomic_load(intptr1);

		std::cout << int1 << std::endl;
		*/
	}
}

void enable_shared_from_this_test()
{
	TEST* num = new TEST(0, 100);

	shared_ptr<TEST> n1(num);
	shared_ptr<TEST> n2(n1->get());

	n1.info();
	n2.info();
}

const int thread_nums = 4;
const int try_numbers = 10000;

shared_ptr<int> world(new int(10000));
std::vector<shared_ptr<int>> vec;

void add_thread_func() {

	shared_ptr<int> p;

	for (int i = 0; i < try_numbers / 4; ++i) {
		
		p = world.get();

	}
}

void sub_thread_func() {

	for (int i = 0; i < try_numbers / 4 ; ++i) {
		auto p = vec.back();
		p.reset();
		vec.pop_back();
	}
}

void atomic_threads_test() {
	
	std::vector<std::thread> threads;

	for (int i = 0; i < thread_nums / 2 ; ++i)
		threads.emplace_back(add_thread_func);

	for (int i = 0; i < thread_nums / 2; ++i)
		threads.emplace_back(sub_thread_func);

	for (auto &th : threads)	th.join();

	world.info();
	std::cout << "vector size : " << vec.size() << std::endl;
}

void main() {
	std::cout << "test start" << std::endl;

	//atomic_threads_test();
	enable_shared_from_this_test();
	//blocking_test();
}