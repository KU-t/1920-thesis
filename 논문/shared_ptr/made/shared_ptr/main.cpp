#include <iostream>
#include "Single-shared_ptr.h"
#include "Single-weak_ptrweak.h"

class character {
private:
	int id;
public:
	character(int _id = 0) : id(_id) {
		std::cout << "create character ID : " << id << std::endl;
	};
	~character() {
		std::cout << "delete characterID : " << id << std::endl;
	};

	void info() { 
		std::cout << "[chararcter ID " << id << "] ";
	}
};

class character1 {
private:
	int id;
	int hp;
public:
	character1(int _id = 0) : id(_id) {
		std::cout << "create character ID : " << id << std::endl;
	};
	~character1() {
		std::cout << "delete characterID : " << id << std::endl;
	};

	void info() {
		std::cout << "[chararcter ID " << id << "] ";
	}
};

void show(shared_ptr<character>& sptr) {
	if (sptr.get() == nullptr)	return;
	sptr->info();
	std::cout << "ref : " << sptr.use_count() << std::endl;
}
void show(shared_ptr<character1>& sptr) {
	if (sptr.get() == nullptr)	return;
	sptr->info();
	std::cout << "ref : " << sptr.use_count() << std::endl;
}

typedef shared_ptr<character> ch_sptr;
typedef shared_ptr<character1> ch1_sptr;

void main() {	
	ch_sptr c0 = new character(0);
	show(c0);

	ch_sptr c1 = c0;
	show(c1);

	ch1_sptr c2;
	c2 = c1;
	show(c2);
	
	c2.reset();
	show(c1);

}