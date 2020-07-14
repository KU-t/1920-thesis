#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>
#include "lockfree_shared_ptr.h"

using namespace LFSP;
using namespace std::chrono;

const auto NUM_TEST = 100000;
const auto KEY_RANGE = 10000;

class SPNODE {
public:
	int key;
	shared_ptr<SPNODE> _next; bool removed;
	std::mutex nlock;

	SPNODE() { _next = nullptr; removed = false; }
	SPNODE(int key_value) { _next = nullptr; key = key_value; removed = false; }

	~SPNODE() {}

	void lock() {
		nlock.lock();
	}

	void unlock() {
		nlock.unlock();
	}
};

class SPCLIST {
	shared_ptr<SPNODE> head, tail;

public:
	SPCLIST() {
		head = make_shared<SPNODE>(0x80000000);
		tail = make_shared<SPNODE>(0x7fffffff);
		head->_next = tail;

		/*std::cout << "[ head ] : " << &head << std::endl;
		std::cout << "[ head ] ptr : " << head.get_ptr() << std::endl;
		std::cout << "[ head ] ctr : " << head.get_ctr() << std::endl;
		std::cout << "[ head ] Use_count : " << head.get_usecount() << std::endl;
		std::cout << "[ head ] Weak_count : " << head.get_weakcount() << std::endl;
		std::cout << "[ head ] _next_ptr : " << head->_next << std::endl;

		std::cout << "[ tail ] : " << &tail << std::endl;
		std::cout << "[ tail ] ptr : " << tail.get_ptr() << std::endl;
		std::cout << "[ tail ] ctr : " << tail.get_ctr() << std::endl;
		std::cout << "[ tail ] Use_count : " << tail.get_usecount() << std::endl;
		std::cout << "[ tail ] Weak_count : " << tail.get_weakcount() << std::endl;
		std::cout << "[ tail ] _next_ptr : " << tail->_next << std::endl;
*/
	}

	~SPCLIST() {
		head = nullptr;
		tail = nullptr;
	}

	void Init() {
		//std::cout << "Clist.Init() ";
		head->_next = tail;
		//std::cout << "Finish" << std::endl;

		/*std::cout << "[ head ] : " << &head << std::endl;
		std::cout << "[ head ] ptr : " << head.get_ptr() << std::endl;
		std::cout << "[ head ] ctr : " << head.get_ctr() << std::endl;
		std::cout << "[ head ] Use_count : " << head.get_usecount() << std::endl;
		std::cout << "[ head ] Weak_count : " << head.get_weakcount() << std::endl;
		std::cout << "[ head ] _next_ptr : " << head->_next << std::endl;

		std::cout << "[ tail ] : " << &tail << std::endl;
		std::cout << "[ tail ] ptr : " << tail.get_ptr() << std::endl;
		std::cout << "[ tail ] ctr : " << tail.get_ctr() << std::endl;
		std::cout << "[ tail ] Use_count : " << tail.get_usecount() << std::endl;
		std::cout << "[ tail ] Weak_count : " << tail.get_weakcount() << std::endl;
		std::cout << "[ tail ] _next_ptr : " << tail->_next << std::endl;*/
	}

	bool validate(const shared_ptr<SPNODE>& pred, const shared_ptr<SPNODE>& curr) {
		return !pred->removed && !curr->removed && pred->_next == curr;
	}

	bool Add(int key) {

		while (true) {
			
			shared_ptr<SPNODE> pred, curr;
			pred = head;
			curr = pred->_next;

			while (curr->key < key) {
				pred = curr;
				//if (curr->_next->key < 0)
				//	DebugBreak();
				curr = curr->_next;
			}
			pred->lock();	curr->lock();

			if (validate(pred, curr)) {
				if (key == curr->key) {
					curr->unlock();	pred->unlock();
					
					return false;
				}
				
				else {
					shared_ptr<SPNODE> node = make_shared<SPNODE>(key);
					
					node->_next = curr;
					pred->_next = node;
					
					curr->unlock();	pred->unlock();
					
					return true;
				}
			}
			curr->unlock();	pred->unlock();
		}
	}

	bool Remove(int key) {

		while (true) {
			shared_ptr<SPNODE> pred, curr;
			pred = head;
			curr = pred->_next;

			while (curr->key < key) {
				pred = curr;
				curr = curr->_next;
			}
			pred->lock();	curr->lock();

			if (validate(pred, curr)) {
				if (key == curr->key) {
					curr->removed = true;
					pred->_next = curr->_next;

					curr->unlock();	pred->unlock();
					
					return true;
				}

				else {
					curr->unlock();	pred->unlock();
					return false;
				}
			}
			curr->unlock();	pred->unlock();
		}
	}

	bool Contains(int key) {
		shared_ptr<SPNODE> curr = head;
		while (curr->key < key) {
			curr = curr->_next;
		}
		return curr->key == key && !curr->removed;
	}

	void display20() {
		int c = 20;
		shared_ptr<SPNODE> p = head->_next;
		while (p != tail) {
			std::cout << p->key << ", ";
			p = p->_next;
			c--;
			if (c == 0)
				return;
		}
	}
};

SPCLIST clist;

void thread_func(int num_thread, int num) {
	int key;
	
	for (int i = 0; i < NUM_TEST / num_thread; i++) {
		if (i < KEY_RANGE) {
			key = rand() % KEY_RANGE;
			//std::cout << "threadNUM : " << num << " | KEY : " << key << std::endl;
			clist.Add(key);
			continue;
		}
		
		switch (rand() % 3) {

		case 0:
			key = rand() % KEY_RANGE;
			clist.Add(key);
			//std::cout << "+" << key << " ";
			break;

		case 1:
			key = rand() % KEY_RANGE;
			clist.Remove(key);
			//std::cout << "-" << key << " ";
			break;

		case 2:
			key = rand() % KEY_RANGE;
			clist.Contains(key);
			break;

		default:
			std::cout << "Error\n";
			exit(-1);
		}
	}
	//std::cout << "Thread [" << num << "] Finish" << std::endl;
}

int main() {
	while(true)
	for (int num_thread = 1; num_thread <= 8; num_thread *= 2) {
		//if(num_thread > 1)
		//	DebugBreak();
		clist.Init();

		std::vector<std::thread> threads;

		auto start_time = high_resolution_clock::now();

		for (int i = 0; i < num_thread; ++i)
			threads.emplace_back(thread_func, num_thread,i);

		for (auto &th : threads)	th.join();

		auto end_time = high_resolution_clock::now();
		auto exec_time = end_time - start_time;
		auto exec_ms = duration_cast<milliseconds>(exec_time).count();

		std::cout << "Threads[" << num_thread << "] \t";
		clist.display20();

		std::cout << "\t Exec_Time : " << exec_ms << "ms" << std::endl;
	}
}