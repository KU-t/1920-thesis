#pragma once

#include <unordered_set>

class Object;
class Player;

class Near_set
{
private:
	std::unordered_set<Object*> set;
	std::shared_mutex mutex;

public:
	std::unordered_set<Object*>::iterator			begin() { return set.begin(); }
	std::unordered_set<Object*>::const_iterator	begin() const { return set.begin(); }
	std::unordered_set<Object*>::iterator			end() { return set.end(); }
	std::unordered_set<Object*>::const_iterator	end()	const { return set.end(); }

public:
	void R_lock();
	void R_unlock();

	void insert(Object* const);
	void insert(const std::vector<Object*>&);

	void erase(Object* const);
	
	bool insert_ifn_find(Object* const);

	void clear();
	void Update(const std::vector<std::pair<Object*, RESULT_MOVE_OBJECT_TYPE>>&);
};

