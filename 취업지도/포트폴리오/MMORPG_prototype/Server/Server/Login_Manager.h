#pragma once

#include <tbb/concurrent_queue.h>

class Login_Manager
{
private:
	tbb::concurrent_queue<Type_ID> queue;

	static Login_Manager* LgnMgr;

	Login_Manager();
public:

	static Login_Manager* Create_Login_Manager();


	Type_ID Pop();
	void Push(const Type_ID&);
};

