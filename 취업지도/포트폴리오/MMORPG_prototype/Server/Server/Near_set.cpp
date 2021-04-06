#include "stdafx.h"
#include "Near_set.h"
#include "Object.h"

void Near_set::R_lock()
{
	mutex.lock_shared();
}

void Near_set::R_unlock()
{
	mutex.unlock_shared();
}

void Near_set::insert(Object* const p_object)
{
	mutex.lock();

	set.insert(p_object);

	mutex.unlock();
}

void Near_set::insert(const std::vector<Object*>& vec_objects)
{
	mutex.lock();

	for (Object* const p_object : vec_objects) 
		set.insert(p_object);
	
	mutex.unlock();
}

void Near_set::erase(Object* const p_erase_object)
{
	mutex.lock();

	for (Object* const p_object : set) {
		if (p_object == p_erase_object) {
			set.erase(p_object);
			mutex.unlock();
			return;
		}
	}

	mutex.unlock();
}

bool Near_set::insert_ifn_find(Object* const p_object)
{
	std::unordered_set<Object*>::iterator iter;
	
	mutex.lock();
	iter = set.find(p_object);

	if (iter == set.end()) {
		set.insert(p_object);
		mutex.unlock();
		return true;
	}

	mutex.unlock();
	return false;
}

void Near_set::clear()
{
	mutex.lock();
	set.clear();
	mutex.unlock();
}

void Near_set::Update(const std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>& vec_objects)
{	
	mutex.lock();

	for (const std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>& vec_element : vec_objects) {
		
		Object* p_object{ vec_element.first };

		if (vec_element.second == RMOT_PLAYER_SIGHT_IN ||
			vec_element.second == RMOT_OBJECT_SIGHT_IN) 
		{
			set.insert(p_object);
		}

		else if (vec_element.second == RMOT_UNEXIST ||
			vec_element.second == RMOT_PLAYER_SIGHT_OUT ||
			vec_element.second == RMOT_OBJECT_SIGHT_OUT)
		{
			set.erase(p_object);
		}
	}

	mutex.unlock();
}





