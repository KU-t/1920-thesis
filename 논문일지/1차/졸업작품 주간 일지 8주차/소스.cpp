#include <iostream>
#include <atomic>
//#include "shared_ptr.h"

//using namespace std;
//using namespace LFSP;


template<typename _Tp>
class shared_ptr
{
public:
	shared_ptr(_Tp* __p)
		: ptr(__p)
	{
		count = new std::atomic_int{ 1 };
	}

	_Tp operator *()
	{
		return *ptr;
	}

	shared_ptr& operator=(shared_ptr& __r) noexcept
	{
		*(__r.count)++;
		count = __r.count;
		ptr = __r.ptr;
		atomic_thread_fence();
		return *this;
	}

	~shared_ptr()
	{
		(*count)--;
		if (0 == (*count)) delete ptr;
	}

	std::atomic_int *count;
	_Tp *ptr;
};



int main() {

	shared_ptr<int> n1 { new int{100} };

	shared_ptr<int> n2 = n1;

	std::cout << *n1 << std::endl;
	std::cout << *n2 << std::endl;

	system("pause");
}