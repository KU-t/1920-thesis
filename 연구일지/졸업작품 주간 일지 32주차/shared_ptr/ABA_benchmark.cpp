#include <iostream>
#include <thread>

#include "memory_circular_queue_shared_ptr.h"

#define THREAD8

class node {
public:
	int val;

	node()
	{}

	node(int _val)	
	{ val = _val; }

	~node()			
	{ val = 0; }

	void show() 
	{
		if(val == 0)
			std::cout << "!" << std::endl;
	}
};

LFSP::shared_ptr<node> g;
LFSP::cb_memory_helper* hp;

void in() {
	int val = 0;

	while (true)
		g = LFSP::make_shared<node>(hp, val++);
}

void out() {
	LFSP::shared_ptr<node> l;


	while (true)
	{
		l = g;
		l->show();
	}
}

void main() {
	hp = new LFSP::cb_memory_helper(1000);

	std::thread i1(in);
	std::thread i2(in);
	std::thread o1(out);
	std::thread o2(out);

#ifdef THREAD8

	std::thread i3(in);
	std::thread i4(in);
	std::thread o3(out);
	std::thread o4(out);

#endif 

	i1.join();
	i2.join();
	o1.join();
	o2.join();
	
#ifdef THREAD8

	i3.join();
	i4.join();
	o3.join();
	o4.join();

#endif
}