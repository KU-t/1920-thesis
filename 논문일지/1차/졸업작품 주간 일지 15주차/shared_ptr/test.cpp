#include <iostream>
#include <thread>
#include <vector>

//#include <memory>
//using namespace std;


//#include "Blocking_shared_ptr.h"
#include "shared_ptr.h"
//using namespace LFSP;

class stdTest : public std::enable_shared_from_this<stdTest>{
public:
	stdTest()
	{}

	stdTest(int input) : data(input)
	{}

	std::shared_ptr<stdTest> get()
	{
		return shared_from_this();
	}

private:
	int data;
};

class lfspTest : public LFSP::enable_shared_from_this<lfspTest> {
public:
	lfspTest()
	{}

	lfspTest(int input) : data(input)
	{}

	LFSP::shared_ptr<lfspTest> get()
	{
		return shared_from_this();
	}

private:
	int data;
};

class lfspTest2{
public:
	lfspTest2()
	{}

	lfspTest2(int input) : data(input)
	{}

private:
	int data;
};


void main() {
	
	std::cout << "std:: make_shared<>Test" << std::endl;
	
	{
		std::cout << "std:: enable_shared_from_this Test" << std::endl;

		{


			std::shared_ptr<int> stdsptr = std::make_shared<int>(10);

			std::cout << "stdsptr<int> : " << stdsptr << std::endl;
			std::cout << stdsptr.use_count() << std::endl;

			std::shared_ptr<int> stdsptr2(stdsptr);

			std::cout << "stdsptr2<int> : " << stdsptr2 << std::endl;
			std::cout << stdsptr2.use_count() << std::endl;
		
		}

		// ---------------------------

		{

			std::shared_ptr<stdTest> stdsptr = std::make_shared<stdTest>(10);

			std::cout << "stdsptr<Test> : " << stdsptr << std::endl;
			std::cout << stdsptr.use_count() << std::endl;

			std::shared_ptr<stdTest> stdsptr2(stdsptr->get());

			std::cout << "stdsptr<Test> : " << stdsptr2 << std::endl;
			std::cout << stdsptr2.use_count() << std::endl;

		}

	}


	{
		std::cout << "LFSP:: enable_shared_from_this Test" << std::endl;


		{


			LFSP::shared_ptr<int> lfspsptr = LFSP::make_shared<int>(10);

			std::cout << "lfspsptr<int> : " << lfspsptr << std::endl;
			std::cout << lfspsptr.use_count() << std::endl;

			LFSP::shared_ptr<int> lfspsptr2(lfspsptr);

			std::cout << "lfspsptr2<int> : " << lfspsptr2 << std::endl;
			std::cout << lfspsptr2.use_count() << std::endl;

		}

		// ---------------------------

		{

			LFSP::shared_ptr<lfspTest> lfspsptr = LFSP::make_shared<lfspTest>(10);

			std::cout << "lfspsptr<Test> : " << lfspsptr << std::endl;
			std::cout << lfspsptr.use_count() << std::endl;

			LFSP::shared_ptr<lfspTest> lfspsptr2(lfspsptr->get());

			std::cout << "lfspsptr2<Test> : " << lfspsptr2 << std::endl;
			std::cout << lfspsptr2.use_count() << std::endl;

		}


		std::cout << "LFSP:: enable_shared_from_this(x) Test" << std::endl;

		{

			LFSP::shared_ptr<lfspTest2> lfspsptr = LFSP::make_shared<lfspTest2>(10);

			std::cout << "lfspsptr<Test> : " << lfspsptr << std::endl;
			std::cout << lfspsptr.use_count() << std::endl;
		}

	}

	{


	}

}