#include <iostream>
#include <thread>
#include <vector>
#include "shared_ptr.h"

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

void function_test() {

	std::cout << std::endl <<  "-----Function Test----- " << std::endl << std::endl;

	
	{

		std::cout << std::endl << "***** shaerd_ptr<> *****" << std::endl << std::endl;


		std::cout << std::endl;
		std::cout << " [ shared_ptr<int> nullptr 积己 ]" << std::endl;
		shared_ptr<int> nptr1(nullptr);
		nptr1.info();


		std::cout << std::endl;
		std::cout << " [ shared_ptr<int> shared_ptr<int>(nullptr) 汗荤积己 ]" << std::endl;
		shared_ptr<int> nptr2(nptr1);
		nptr2.info();


		/*
		std::cout << "[ shared_ptr<int> () 积己  ] " << std::endl;
		shared_ptr<int> b();		// *********************
		//b.info();
		*/

		std::cout << std::endl;
		std::cout << " [ shared_ptr<int> new int() 2俺 积己 ]" << std::endl;
		shared_ptr<int> intptr1(new int(10));
		shared_ptr<int> intptr2(new int(100));
		intptr1.info();
		intptr2.info();


		std::cout << std::endl;
		std::cout << " [ shared_ptr<int> shared_ptr<int>(int) 汗荤积己 ]" << std::endl;
		shared_ptr<int> intptr3(intptr1);
		intptr1.info();
		intptr2.info();


		std::cout << std::endl;
		std::cout << "[ shared_ptr<int> shared_ptr<int> 措涝 ]" << std::endl;
		intptr3 = intptr2;
		intptr1.info();
		intptr2.info();


		std::cout << std::endl;
		std::cout << " [ shared_ptr<class> shared_ptr<class>(nullptr) 积己 ] " << std::endl;
		shared_ptr<TEST> ntestptr;
		ntestptr.info();

		
		std::cout << std::endl;
		std::cout << " [ shared_ptr<class> new class() 2俺 积己 ] " << std::endl;
		shared_ptr<TEST> testptr1(new TEST());
		shared_ptr<TEST> testptr2(new TEST(10, 20));
		testptr1.info();
		testptr2.info();


		std::cout << std::endl;
		std::cout << " [ shared_ptr<class> shared_ptr<class> 措涝 ] " << std::endl;
		shared_ptr<TEST> testptr3(testptr1);
		testptr1 = testptr2;
		testptr1.info();
		testptr2.info();
		testptr3.info();


		std::cout << std::endl;
		std::cout << " [ shared_ptr<class> shared_ptr<class> == & use_count() ] " << std::endl;
		std::cout << "testptr1 use_count : " << testptr1.use_count() << std::endl;
		std::cout << "testptr2 use_count : " << testptr2.use_count() << std::endl;
		std::cout << "testptr3 use_count : " << testptr3.use_count() << std::endl;
		std::cout << "testptr1 == testptr2 : " << std::boolalpha << (testptr1 == testptr2) << std::endl;
		std::cout << "testptr3 == nullptr : " << std::boolalpha << (testptr3 == nullptr) << std::endl;
		std::cout << "nullptr == testptr3 : " << std::boolalpha << (nullptr == testptr3) << std::endl;
		std::cout << "testptr1 != testptr2 : " << std::boolalpha << (testptr1 != testptr2) << std::endl;
		std::cout << "testptr3 != nullptr : " << std::boolalpha << (testptr3 != nullptr) << std::endl;
		std::cout << "nullptr != testptr3 : " << std::boolalpha << (nullptr != testptr3) << std::endl;


		std::cout << std::endl;
		std::cout << " [ shared_ptr<int> shared_ptr<class> get() ] " << std::endl;
		int* intp(intptr1.get());
		TEST* testp(testptr1.get());
		std::cout << *intp << std::endl;
		testp->info();


		std::cout << std::endl;
		std::cout << " [ shared_ptr<int> shared_ptr<class> bool() ] " << std::endl;
		std::cout << "nptr operator bool() : ";
		std::cout << std::boolalpha << (nptr1.operator bool()) << std::endl;
		std::cout << "testptr1 operator bool() : ";
		std::cout << (testptr1.operator bool()) << std::noboolalpha << std::endl;


		std::cout << std::endl;
		std::cout << " [ shared_ptr<int> operator*,  shared_ptr<class> -> ] " << std::endl;
		std::cout << *intptr1 << std::endl;
		std::cout << *intptr2 << std::endl;
		testptr1.info();
		testptr1->info();

	}
	


	{
		std::cout << std::endl << std::endl << 
			"***** weak_ptr<> *****" << std::endl << std::endl;


		std::cout << std::endl;
		std::cout << " [ weak_ptr<int> nullptr 积己 ]" << std::endl;
		weak_ptr<int> nptr1(nullptr);
		nptr1.info();


		std::cout << std::endl;
		std::cout << " [ weak_ptr<int> weak_ptr<int>(nullptr) 汗荤积己 ]" << std::endl;
		weak_ptr<int> nptr2(nptr1);
		nptr2.info();


		std::cout << std::endl;
		std::cout << " [ shared_ptr<int> weak_ptr<int> 积己] " << std::endl;
		shared_ptr<int> intptr1(new int(10));
		shared_ptr<int> intptr2(new int(100));
		weak_ptr<int> weakintptr1(intptr1);
		weak_ptr<int> weakintptr2(intptr2);
		weak_ptr<int> weakintptr3(intptr2);
		weakintptr1.info();
		weakintptr2.info();
		weakintptr3.info();

		std::cout << std::endl;
		std::cout << " [ weak_ptr<int> weak_ptr<int> 汗荤积己] " << std::endl;
		weak_ptr<int> weakintptr4(weakintptr1);
		weakintptr4.info();

		std::cout << std::endl;
		std::cout << " [ weak_ptr<int> weak_ptr<int> 措涝] " << std::endl;
		weakintptr4 = weakintptr2;
		weakintptr1.info();
		weakintptr2.info();

		std::cout << std::endl;
		std::cout << " [ weak_ptr<int> shared_ptr<int> 措涝] " << std::endl;
		weakintptr4 = intptr1;
		weakintptr1.info();
		weakintptr2.info();

	}

	{
		std::cout << std::endl << std::endl <<
			"***** weak_ptr<> ------ shared_ptr<> *****" << std::endl << std::endl;


		std::cout << std::endl;
		std::cout << "[shared_ptr<int> 积己 ]" << std::endl;
		shared_ptr<int> shared_weak_ptr1(new int(1000));
		shared_ptr<int> shared_weak_ptr2(new int(10000));
		shared_weak_ptr1.info();
		shared_weak_ptr2.info();


		std::cout << std::endl;
		std::cout << "[ weak_ptr<int> shared_ptr<int> 积己 ]" << std::endl;
		weak_ptr<int> weak_shared_ptr1(shared_weak_ptr1);
		weak_ptr<int> weak_shared_ptr2(shared_weak_ptr2);
		weak_shared_ptr1.info();
		weak_shared_ptr2.info();


		std::cout << std::endl;
		std::cout << "[ shared_ptr<int> weak_ptr<int> 积己 ]" << std::endl;
		shared_ptr<int> shared_weak_ptr3(weak_shared_ptr1);
		shared_ptr<int> shared_weak_ptr4(weak_shared_ptr2);
		weak_shared_ptr1.info();
		weak_shared_ptr2.info();


		std::cout << std::endl;
		std::cout << "[ shared_ptr<int> weak_ptr<int> 措涝 ]" << std::endl;
		shared_weak_ptr1 = weak_shared_ptr2;
		weak_shared_ptr1.info();
		weak_shared_ptr2.info();


		std::cout << std::endl << std::endl <<
			"***** weak_ptr<> ------ shared_ptr<> *****" << std::endl << std::endl;
		weak_shared_ptr1 = shared_weak_ptr2;
		shared_weak_ptr3.info();
		shared_weak_ptr4.info();
	}


	{
		std::cout << std::endl << std::endl <<
			"***** make_shared<> *****" << std::endl << std::endl;
		shared_ptr<int> make_shared_ptr1(make_shared<int>(1));
		shared_ptr<int> make_shared_ptr2 = make_shared<int>(2);
		make_shared_ptr1.info();
		make_shared_ptr2.info();

	}


	std::cout << std::endl;
	std::cout << "-----end-----" << std::endl;
}




const int thread_nums = 4;
shared_ptr<int> world(new int(10));
std::vector<shared_ptr<int>> vec[thread_nums];

void function_thread1(int index) {

	int try_func = 1'000;
	
	for (int i = 0; i < try_func; ++i) {
		vec[index].emplace_back(shared_ptr<int>(world));
	}
}

void function_thread2(int index) {

	int try_func = 1'000;

	for (int i = 0; i < try_func; ++i) {
		auto p = vec[index].back();
		p.reset();
		vec[index].pop_back();
	}
}

void ref_test() {

	std::vector<std::thread> threads;

	for (int i = 0; i < thread_nums; ++i) 
		threads.emplace_back(function_thread1, i);
	
	for (auto &th : threads)	th.join();
	
	world.info();



	threads.clear();

	for (int i = 0; i < thread_nums; ++i)
		threads.emplace_back(function_thread2, i);

	for (auto &th : threads)	th.join();

	world.info();
}

void enable_shared_from_this_test()
{
	TEST* num = new TEST(0, 100);

	shared_ptr<TEST> n1(num);
	shared_ptr<TEST> n2 = n1->get();

	n1.info();
	n2.info();
}

void main() {

	//function_test();
	//ref_test();
	enable_shared_from_this_test();
	
}