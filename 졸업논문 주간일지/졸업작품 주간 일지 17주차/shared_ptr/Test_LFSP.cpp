#include <memory>
#include <iostream>
#include "Blocking_shared_ptr.h"

class TEST 
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

	std::shared_ptr<TEST> get()
	{
		return std::shared_ptr<TEST>(this);
		//return shared_from_this();
	}

private:
	int ID;
	int HP;
};

class lfspTEST : public LFSP::enable_shared_from_this<lfspTEST>
{
public:
	void info() //***********test
	{
		std::cout << "ID : " << ID << " / HP : " << HP << std::endl;
	}

	lfspTEST()
		: ID(0), HP(0)
	{}

	lfspTEST(int _ID, int _HP)
		: ID(_ID), HP(_HP)
	{}

	LFSP::shared_ptr<lfspTEST> get()
	{
		//return shared_ptr<TEST>(this);
		return shared_from_this();
	}

private:
	int ID;
	int HP;
};

class stdTEST : public std::enable_shared_from_this<stdTEST>
{
public:
	void info() //***********test
	{
		std::cout << "ID : " << ID << " / HP : " << HP << std::endl;
	}

	stdTEST()
		: ID(0), HP(0)
	{}

	stdTEST(int _ID, int _HP)
		: ID(_ID), HP(_HP)
	{}

	std::shared_ptr<stdTEST> copy_shared()
	{
		//return shared_ptr<TEST>(this);
		return shared_from_this();
	}

private:
	int ID;
	int HP;
};

int main() {

	{
		// 1. make_shared<int>
		std::cout << "\n\n[1 TEST]\n";
		std::shared_ptr<int> ptr = std::make_shared<int>(10);
		LFSP::shared_ptr<int> _ptr = LFSP::make_shared<int>(10);
	}	
		
	{	
		// 2. make_shared<T>
		std::cout << "\n\n[2 TEST]\n";
		std::shared_ptr<TEST> ptr = std::make_shared<TEST>(1,10);
		LFSP::shared_ptr<TEST> _ptr = LFSP::make_shared<TEST>(1, 10);
	}	
		
	{	
		// 3.make_shared<enable_from_T>=
		std::cout << "\n\n[3 TEST]\n";
		std::shared_ptr<stdTEST> ptr = std::make_shared<stdTEST>(1, 10);
		LFSP::shared_ptr<lfspTEST> _ptr = LFSP::make_shared<lfspTEST>(1, 10);
	}	
		
	{	
		// shared_ptr(ptr(int))	//***************************
		//int* ptr = new int(10);
		//std::shared_ptr<int> ptr_c1(ptr);
		//std::shared_ptr<int> ptr_c2(ptr);
		//int* _ptr = new int(10);
		//LFSP::shared_ptr<int> _ptr_c1(_ptr);
		//LFSP::shared_ptr<int> _ptr_c2(_ptr);
	}	
		
	{	
		// shared_ptr(ptr(T))	//***************************
		//TEST* ptr = new TEST(1, 10);
		//std::shared_ptr<TEST> ptr_c1(ptr);
		//std::shared_ptr<TEST> ptr_c2(ptr);
		//TEST* _ptr = new TEST(1, 10);
		//LFSP::shared_ptr<TEST> _ptr_c1(_ptr);
		//LFSP::shared_ptr<TEST> _ptr_c2(_ptr);
	}	
		
	{	
		// shared_ptr(ptr(enable_from_T))	//***************************
		//stdTEST* ptr = new stdTEST(1, 10);
		//std::shared_ptr<stdTEST> ptr_c1(ptr);
		//std::shared_ptr<stdTEST> ptr_c2(ptr);
		//lfspTEST* _ptr = new lfspTEST(1, 10);
		//LFSP::shared_ptr<lfspTEST> _ptr_c1(_ptr);
		//LFSP::shared_ptr<lfspTEST> _ptr_c2(_ptr);	
	}	
		
	{	
		// 4. shared_ptr(shared_ptr<int>)
		std::cout << "\n\n[4 TEST]\n";
		std::shared_ptr<int> ptr = std::make_shared<int>(10);
		std::shared_ptr<int> ptr_c(ptr);
		LFSP::shared_ptr<int> _ptr = LFSP::make_shared<int>(10);
		LFSP::shared_ptr<int> _ptr_c(_ptr);
	}	
		
	{	
		// 5. shared_ptr(shared_ptr<T>)
		std::cout << "\n\n[5 TEST]\n";
		std::shared_ptr<TEST> ptr = std::make_shared<TEST>(1,10);
		std::shared_ptr<TEST> ptr_c(ptr);
		LFSP::shared_ptr<TEST> _ptr = LFSP::make_shared<TEST>(1,10);
		LFSP::shared_ptr<TEST> _ptr_c(_ptr);
	}	
		
	{	
		// 6. shared_ptr(shared_ptr<enable_from_T>)
		std::cout << "\n\n[6 TEST]\n";
		std::shared_ptr<stdTEST> ptr = std::make_shared<stdTEST>(1, 10);
		std::shared_ptr<stdTEST> ptr_c(ptr);
		LFSP::shared_ptr<lfspTEST> _ptr = LFSP::make_shared<lfspTEST>(1, 10);
		LFSP::shared_ptr<lfspTEST> _ptr_c(_ptr);
	}	
		
	{	
		// 7. shared_ptr(weak_ptr<int>)
		std::cout << "\n\n[7 TEST]\n";
		std::shared_ptr<int> ptr = std::make_shared<int>(10);
		std::weak_ptr<int> ptr_wc(ptr);
		std::shared_ptr<int> ptr_c(ptr_wc);
		LFSP::shared_ptr<int> _ptr = LFSP::make_shared<int>(10);
		LFSP::weak_ptr<int> _ptr_wc(_ptr);
		LFSP::shared_ptr<int> _ptr_c(_ptr);
	}	
		
	{	
		// 8. shared_ptr(weak_ptr<T>)
		std::cout << "\n\n[8 TEST]\n";
		std::shared_ptr<TEST> ptr = std::make_shared<TEST>(1, 10);
		std::weak_ptr<TEST> ptr_wc(ptr);
		std::shared_ptr<TEST> ptr_c(ptr_wc);
		LFSP::shared_ptr<TEST> _ptr = LFSP::make_shared<TEST>(1, 10);
		LFSP::weak_ptr<TEST> _ptr_wc(_ptr);
		LFSP::shared_ptr<TEST> _ptr_c(_ptr);
	}	
		
	{	
		// 9. shared_ptr(weak_ptr<enable_from_T>) 
		std::cout << "\n\n[9 TEST]\n";
		std::shared_ptr<stdTEST> ptr = std::make_shared<stdTEST>(1, 10);
		std::weak_ptr<stdTEST> ptr_wc(ptr);
		std::shared_ptr<stdTEST> ptr_c(ptr_wc);
		LFSP::shared_ptr<lfspTEST> _ptr = LFSP::make_shared<lfspTEST>(1, 10);
		LFSP::weak_ptr<lfspTEST> _ptr_wc(_ptr);
		LFSP::shared_ptr<lfspTEST> _ptr_c(_ptr);
	}	
		
	{	
		// 10. shared_ptr = shared_ptr<int>
		std::cout << "\n\n[10 TEST]\n";
		std::shared_ptr<int> ptr = std::make_shared<int>(1);
		std::shared_ptr<int> ptr_ = std::make_shared<int>(10);
		ptr = ptr_;
		LFSP::shared_ptr<int> _ptr = LFSP::make_shared<int>(1);
		LFSP::shared_ptr<int> _ptr_ = LFSP::make_shared<int>(10);
		_ptr = _ptr_;
	}	
		
	{	
		
		// 11. shared_ptr = shared_ptr<T>	
		std::cout << "\n\n[11 TEST]\n";
		std::shared_ptr<TEST> ptr = std::make_shared<TEST>(1, 10);
		std::shared_ptr<TEST> ptr_ = std::make_shared<TEST>(10, 100);
		ptr = ptr_;
		LFSP::shared_ptr<TEST> _ptr = LFSP::make_shared<TEST>(1, 10);
		LFSP::shared_ptr<TEST> _ptr_ = LFSP::make_shared<TEST>(10, 100);
		_ptr = _ptr_;
	}	
		
	{	
		// 12. shared_ptr = shared_ptr<enable_from_T>
		std::cout << "\n\n[12 TEST]\n";
		std::shared_ptr<stdTEST> ptr = std::make_shared<stdTEST>(1, 10);
		std::shared_ptr<stdTEST> ptr_ = std::make_shared<stdTEST>(10, 100);
		ptr = ptr_;
		LFSP::shared_ptr<lfspTEST> _ptr = LFSP::make_shared<lfspTEST>(1, 10);
		LFSP::shared_ptr<lfspTEST> _ptr_ = LFSP::make_shared<lfspTEST>(10, 100);
		_ptr = _ptr_;
	}	
		
	{	
		// 13. operator* & operator->
		std::cout << "\n\n[13 TEST]\n";
		std::shared_ptr<stdTEST> ptr = std::make_shared<stdTEST>(1, 10);
		ptr->info();
		(*ptr).info();
		LFSP::shared_ptr<lfspTEST> _ptr = LFSP::make_shared<lfspTEST>(1, 10);
		_ptr->info();
		(*_ptr).info();
	}	
		
	{	
		// 14. operator== & operator!=
		std::cout << "\n\n[14 TEST]\n";
		std::shared_ptr<stdTEST> ptr = std::make_shared<stdTEST>(1, 10);
		std::shared_ptr<stdTEST> ptr_c(ptr);
		std::shared_ptr<stdTEST> ptr_ = std::make_shared<stdTEST>(10, 100);
		std::cout << (ptr == ptr_c) << std::endl;
		std::cout << (ptr != ptr_) << std::endl;
		
		LFSP::shared_ptr<lfspTEST> _ptr = LFSP::make_shared<lfspTEST>(1, 10);
		LFSP::shared_ptr<lfspTEST> _ptr_c(_ptr);
		LFSP::shared_ptr<lfspTEST> _ptr_ = LFSP::make_shared<lfspTEST>(10, 100);
		std::cout << (_ptr == _ptr_c) << std::endl;
		std::cout << (_ptr != _ptr_) << std::endl;
	}	
		
	{	
		// 15. [method] reset()	
		std::cout << "\n\n[15 TEST]\n";
		std::shared_ptr<stdTEST> ptr = std::make_shared<stdTEST>(1, 10);
		ptr.reset();
		LFSP::shared_ptr<lfspTEST> _ptr = LFSP::make_shared<lfspTEST>(1, 10);
		_ptr.reset();
	}	
		
	{	
		
		// 16. [method] use_count()	
		std::cout << "\n\n[16 TEST]\n";
		std::shared_ptr<stdTEST> ptr = std::make_shared<stdTEST>(1, 10);
		std::cout << ptr.use_count() << std::endl;
		std::shared_ptr<stdTEST> ptr_c(ptr);
		std::cout << ptr.use_count() << std::endl;
		ptr.reset();
		std::cout << ptr.use_count() << std::endl;
		LFSP::shared_ptr<lfspTEST> _ptr = LFSP::make_shared<lfspTEST>(1, 10);
		std::cout << _ptr.use_count() << std::endl;
		LFSP::shared_ptr<lfspTEST> _ptr_c(_ptr);
		std::cout << _ptr.use_count() << std::endl;
		_ptr.reset();
		std::cout << ptr.use_count() << std::endl;
	}	
		
	{	
		
		// 17. [method] swap()	
		std::cout << "\n\n[17 TEST]\n";
		std::shared_ptr<stdTEST> ptr = std::make_shared<stdTEST>(1, 10);
		std::shared_ptr<stdTEST> ptr_ = std::make_shared<stdTEST>(10, 100);
		ptr.swap(ptr_);
		LFSP::shared_ptr<lfspTEST> _ptr = LFSP::make_shared<lfspTEST>(1, 10);
		LFSP::shared_ptr<lfspTEST> _ptr_ = LFSP::make_shared<lfspTEST>(10, 100);
		_ptr.swap(_ptr_);
	}	
		
		
		
	{	
		// 18. weak_ptr(shared_ptr<int>)
		std::cout << "\n\n[18 TEST]\n";
		std::shared_ptr<int> ptr = std::make_shared<int>(10);
		std::weak_ptr<int> ptr_c(ptr);
		LFSP::shared_ptr<int> _ptr = LFSP::make_shared<int>(10);
		LFSP::weak_ptr<int> _ptr_c(_ptr);
	}	
		
	{	
		// 19. weak_ptr(shared_ptr<T>)
		std::cout << "\n\n[19 TEST]\n";
		std::shared_ptr<TEST> ptr = std::make_shared<TEST>(1,10);
		std::weak_ptr<TEST> ptr_c(ptr);
		LFSP::shared_ptr<TEST> _ptr = LFSP::make_shared<TEST>(1,10);
		LFSP::weak_ptr<TEST> _ptr_c(_ptr);
	}	
		
	{	
		// 20. weak_ptr(shared_ptr<enable_from_T>)	
		std::cout << "\n\n[20 TEST]\n";
		std::shared_ptr<stdTEST> ptr = std::make_shared<stdTEST>(1, 10);
		std::weak_ptr<stdTEST> ptr_c(ptr);
		LFSP::shared_ptr<lfspTEST> _ptr = LFSP::make_shared<lfspTEST>(1, 10);
		LFSP::weak_ptr<lfspTEST> _ptr_c(_ptr);
	}	
		
	{	
		// 21. weak_ptr(weak_ptr<int>)
		std::cout << "\n\n[21 TEST]\n";
		std::shared_ptr<int> ptr = std::make_shared<int>(10);
		std::weak_ptr<int> ptr_c(ptr);
		std::weak_ptr<int> ptr_wc(ptr_c);
		LFSP::shared_ptr<int> _ptr = LFSP::make_shared<int>(10);
		LFSP::weak_ptr<int> _ptr_c(_ptr);
		LFSP::weak_ptr<int> _ptr_wc(_ptr_c);
	}	
		
	{	
		
		// 22. weak_ptr(weak_ptr<T>)
		std::cout << "\n\n[22 TEST]\n";
		std::shared_ptr<TEST> ptr = std::make_shared<TEST>(1,10);
		std::weak_ptr<TEST> ptr_c(ptr);
		std::weak_ptr<TEST> ptr_wc(ptr_c);
		LFSP::shared_ptr<TEST> _ptr = LFSP::make_shared<TEST>(1,10);
		LFSP::weak_ptr<TEST> _ptr_c(_ptr);
		LFSP::weak_ptr<TEST> _ptr_wc(_ptr_c);
	}	
		
	{	
		
		// 23. weak_ptr(weak_ptr<enable_from_T>)	
		std::cout << "\n\n[23 TEST]\n";
		std::shared_ptr<stdTEST> ptr = std::make_shared<stdTEST>(1, 10);
		std::weak_ptr<stdTEST> ptr_c(ptr);
		std::weak_ptr<stdTEST> ptr_wc(ptr_c);
		LFSP::shared_ptr<lfspTEST> _ptr = LFSP::make_shared<lfspTEST>(1, 10);
		LFSP::weak_ptr<lfspTEST> _ptr_c(_ptr);
		LFSP::weak_ptr<lfspTEST> _ptr_wc(_ptr_c);
		
	}	
		
	{	
		// 24. weak_ptr = shaerd_ptr<int>
		std::cout << "\n\n[24 TEST]\n";
		std::shared_ptr<int> ptr = std::make_shared<int>(10);
		std::weak_ptr<int> ptr_w;
		ptr_w = ptr;
		LFSP::shared_ptr<int> _ptr = LFSP::make_shared<int>(10);
		LFSP::weak_ptr<int> _ptr_w;
		_ptr_w = _ptr;
	}	
		
	{	
		// 25. weak_ptr = shaerd_ptr<T>
		std::cout << "\n\n[25 TEST]\n";
		std::shared_ptr<TEST> ptr = std::make_shared<TEST>(1, 10);
		std::weak_ptr<TEST> ptr_w;
		ptr_w = ptr;
		LFSP::shared_ptr<TEST> _ptr = LFSP::make_shared<TEST>(1, 10);
		LFSP::weak_ptr<TEST> _ptr_w;
		_ptr_w = _ptr;
	}	
		
	{	
		// 26. weak_ptr = shaerd_ptr<enable_from_T>	
		std::cout << "\n\n[26 TEST]\n";
		std::shared_ptr<stdTEST> ptr = std::make_shared<stdTEST>(1, 10);
		std::weak_ptr<stdTEST> ptr_w;
		ptr_w = ptr;
		LFSP::shared_ptr<lfspTEST> _ptr = LFSP::make_shared<lfspTEST>(1, 10);
		LFSP::weak_ptr<lfspTEST> _ptr_w;
		_ptr_w = _ptr;
	}	
		
	{	
		// 27. weak_ptr = weak_ptr<int>
		std::cout << "\n\n[27 TEST]\n";
		std::shared_ptr<int> ptr = std::make_shared<int>(10);
		std::weak_ptr<int> ptr_wc(ptr);
		std::weak_ptr<int> ptr_w;
		ptr_w = ptr_wc;
		LFSP::shared_ptr<int> _ptr = LFSP::make_shared<int>(10);
		LFSP::weak_ptr<int> _ptr_wc(_ptr);
		LFSP::weak_ptr<int> _ptr_w;
		_ptr_w = _ptr_wc;
		
	}
		
	{	
		// 28. weak_ptr = weak_ptr<T>
		std::cout << "\n\n[28 TEST]\n";
		std::shared_ptr<TEST> ptr = std::make_shared<TEST>(1, 10);
		std::weak_ptr<TEST> ptr_wc(ptr);
		std::weak_ptr<TEST> ptr_w;
		ptr_w = ptr_wc;
		LFSP::shared_ptr<TEST> _ptr = LFSP::make_shared<TEST>(1, 10);
		LFSP::weak_ptr<TEST> _ptr_wc(_ptr);
		LFSP::weak_ptr<TEST> _ptr_w;
		_ptr_w = _ptr_wc;
	}	
		
	{	
		// 29. weak_ptr = weak_ptr<enable_from_T>
		std::cout << "\n\n[29 TEST]\n";
		std::shared_ptr<stdTEST> ptr = std::make_shared<stdTEST>(1, 10);
		std::weak_ptr<stdTEST> ptr_wc(ptr);
		std::weak_ptr<stdTEST> ptr_w;
		ptr_w = ptr_wc;
		LFSP::shared_ptr<lfspTEST> _ptr = LFSP::make_shared<lfspTEST>(1, 10);
		LFSP::weak_ptr<lfspTEST> _ptr_wc(_ptr);
		LFSP::weak_ptr<lfspTEST> _ptr_w;
		_ptr_w = _ptr_wc;
	}

	{
		// 30. [method] reset()
		std::cout << "\n\n[30 TEST]\n";
		std::shared_ptr<stdTEST> ptr = std::make_shared<stdTEST>(1, 10);
		std::weak_ptr<stdTEST> ptr_w(ptr);
		ptr_w.reset();
		LFSP::shared_ptr<lfspTEST> _ptr = LFSP::make_shared<lfspTEST>(1, 10);
		LFSP::weak_ptr<lfspTEST> _ptr_w(_ptr);
		_ptr_w.reset();
	}

	{
		// 31. [method] swap()
		std::cout << "\n\n[31 TEST]\n";
		std::shared_ptr<stdTEST> ptr = std::make_shared<stdTEST>(1, 10);
		std::weak_ptr<stdTEST> ptr_w(ptr);
		std::shared_ptr<stdTEST> ptr_ = std::make_shared<stdTEST>(10, 100);
		std::weak_ptr<stdTEST> ptr_w_(ptr_);
		ptr_w.swap(ptr_w_);
		LFSP::shared_ptr<lfspTEST> _ptr = LFSP::make_shared<lfspTEST>(1, 10);
		LFSP::weak_ptr<lfspTEST> _ptr_w(_ptr);
		LFSP::shared_ptr<lfspTEST> _ptr_ = LFSP::make_shared<lfspTEST>(10, 100);
		LFSP::weak_ptr<lfspTEST> _ptr_w_(_ptr_);
		_ptr_w.swap(_ptr_w_);
	}
}