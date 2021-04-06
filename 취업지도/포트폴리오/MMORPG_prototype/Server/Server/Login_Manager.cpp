#include "stdafx.h"
#include <iostream>

#include "Login_Manager.h"

Login_Manager* Login_Manager::LgnMgr = nullptr;

Login_Manager::Login_Manager()
{
	for (int i = 0; i < MAX_PLAYER; ++i)
		queue.push(i);
}

Login_Manager* Login_Manager::Create_Login_Manager()
{
	if (LgnMgr == nullptr) {
		LgnMgr = new Login_Manager;
		printf("[Login_Manager] - Create \n");
	}
	return LgnMgr;
}

Type_ID Login_Manager::Pop()
{
	if (queue.empty()) 	return -1;
	
	Type_ID id;
	while (false == queue.try_pop(id)) {
		if (queue.empty())	return -1;
	}
		
	return id;
}

void Login_Manager::Push(const Type_ID& id)
{
	queue.push(id);
}