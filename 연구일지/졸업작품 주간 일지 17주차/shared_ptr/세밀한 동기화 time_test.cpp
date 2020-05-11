#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>
#include "blocking_shared_ptr.h"

using namespace LFSP;
using namespace std::chrono;

const auto NUM_TEST = 100000;
const auto KEY_RANGE = 1000;

class LFSPN : public enable_shared_from_this<LFSPN> {
public:
	int key;
	shared_ptr<LFSPN> next;
	std::mutex nlock;
	LFSPN() { next = nullptr; }
	LFSPN(int key_value) { next = nullptr; key = key_value; }
	~LFSPN() {}

	void lock() {
		nlock.lock();
	}

	void unlock() {
		nlock.unlock();
	}
};

class LFSPLIST {
	shared_ptr<LFSPN> head, tail;
	int remove_cnt;

public:
	LFSPLIST() {
		head = make_shared<LFSPN>(0x80000000);
		tail = make_shared<LFSPN>(0x7fffffff);
		head->next = tail;
	}

	~LFSPLIST() {}

	void Init() {
		head->next = tail;
	}

	bool Add(int key) {

		shared_ptr<LFSPN> pred, curr;
		head->lock();
		pred = head;
		curr = pred->next;

		curr->lock();

		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (key == curr->key) {
			curr->unlock();
			pred->unlock();
			return false;
		}

		else {
			shared_ptr<LFSPN> add_node = make_shared<LFSPN>(key);
			add_node->next = curr;
			pred->next = add_node;
			curr->unlock();
			pred->unlock();
			return true;
		}
	}

	bool Remove(int key) {

		shared_ptr<LFSPN> pred, curr;
		head->lock();
		pred = head;
		curr = pred->next;

		curr->lock();

		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (key == curr->key) {
			pred->next = curr->next;
			curr->unlock();

			pred->unlock();

			remove_cnt++;

			return true;
		}

		else {
			curr->unlock();
			pred->unlock();
			return false;
		}
	}

	bool Contains(int key) {
		shared_ptr<LFSPN> pred, curr;
		head->lock();
		pred = head;
		curr = pred->next;

		curr->lock();

		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (key == curr->key) {
			curr->unlock();
			pred->unlock();
			return true;
		}

		else {
			curr->unlock();
			pred->unlock();
			return false;
		}
	}

	void display20() {
		int c = 20;
		shared_ptr<LFSPN> p = head->next;

		//std::cout << "remove count : " << remove_cnt << std::endl;
		//remove_cnt = 0;

		while (p != tail) {
			std::cout << p->key << ", ";
			p = p->next;
			c--;
			if (c == 0)
				return;
		}
	}
};

class NODE : public std::enable_shared_from_this<NODE> {
public:
	int key;
	std::shared_ptr<NODE> next;
	std::mutex nlock;
	NODE() { next = nullptr; }
	NODE(int key_value) { next = nullptr; key = key_value; }
	~NODE() {}

	void lock() {
		nlock.lock();
	}

	void unlock() {
		nlock.unlock();
	}
};

class CLIST {
	std::shared_ptr<NODE> head, tail;
public:
	CLIST() {
		head = std::make_shared<NODE>(0x80000000);
		tail = std::make_shared<NODE>(0x7fffffff);
		head->next = tail;
	}

	~CLIST() {}

	void Init() {
		//std::cout << "Clist.Init()" << std::endl;
		head->next = tail;
	}

	bool Add(int key) {

		std::shared_ptr<NODE> pred, curr;
		head->lock();
		pred = head;
		curr = pred->next;

		curr->lock();

		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (key == curr->key) {
			curr->unlock();
			pred->unlock();
			return false;
		}

		else {
			std::shared_ptr<NODE> add_node = std::make_shared<NODE>(key);
			add_node->next = curr;
			pred->next = add_node;
			curr->unlock();
			pred->unlock();
			return true;
		}
	}

	bool Remove(int key) {
		std::shared_ptr<NODE> pred, curr;
		head->lock();
		pred = head;
		curr = pred->next;

		curr->lock();

		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (key == curr->key) {
			pred->next = curr->next;
			curr->unlock();

			pred->unlock();
			return true;
		}

		else {
			curr->unlock();
			pred->unlock();
			return false;
		}
	}

	bool Contains(int key) {
		std::shared_ptr<NODE> pred, curr;
		head->lock();
		pred = head;
		curr = pred->next;

		curr->lock();

		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (key == curr->key) {
			curr->unlock();
			pred->unlock();
			return true;
		}

		else {
			curr->unlock();
			pred->unlock();
			return false;
		}
	}

	void display20() {
		int c = 20;
		std::shared_ptr<NODE> p = head->next;
		while (p != tail) {
			std::cout << p->key << ", ";
			p = p->next;
			c--;
			if (c == 0)
				return;
		}
	}
};

class ATOMICN {

	std::shared_ptr<NODE> head, tail;
public:
	ATOMICN() {
		head = std::make_shared<NODE>(0x80000000);
		tail = std::make_shared<NODE>(0x7fffffff);

		head->next = tail;
	}

	~ATOMICN() {}

	void Init() {
		head->next = tail;
	}

	bool Add(int key) {

		std::shared_ptr<NODE> pred, curr;
		head->lock();

		std::shared_ptr<NODE> empty = atomic_load(&head);
		atomic_store(&pred, empty);

		empty = atomic_load(&pred->next);
		atomic_store(&curr, empty);

		curr->lock();

		while (curr->key < key) {
			pred->unlock();

			empty = atomic_load(&curr);
			atomic_store(&pred, empty);

			empty = atomic_load(&curr->next);
			atomic_store(&curr, empty);

			curr->lock();
		}

		if (key == curr->key) {
			curr->unlock();
			pred->unlock();
			return false;
		}

		else {
			std::shared_ptr<NODE> add_node = std::make_shared<NODE>(key);

			empty = atomic_load(&curr);
			add_node->next = empty;

			empty = atomic_load(&add_node);
			atomic_store(&pred->next, empty);

			curr->unlock();
			pred->unlock();
			return true;
		}
	}

	bool Remove(int key) {
		std::shared_ptr<NODE> pred, curr;
		head->lock();

		std::shared_ptr<NODE> empty = atomic_load(&head);
		atomic_store(&pred, empty);

		empty = atomic_load(&pred->next);
		atomic_store(&curr, empty);

		curr->lock();

		while (curr->key < key) {
			pred->unlock();

			empty = atomic_load(&curr);
			atomic_store(&pred, empty);

			empty = atomic_load(&curr->next);
			atomic_store(&curr, empty);

			curr->lock();
		}

		if (key == curr->key) {
			empty = atomic_load(&curr->next);
			atomic_store(&pred->next, empty);

			curr->unlock();

			pred->unlock();
			return true;
		}

		else {
			curr->unlock();
			pred->unlock();
			return false;
		}
	}

	bool Contains(int key) {
		std::shared_ptr<NODE> pred, curr;
		head->lock();

		std::shared_ptr<NODE> empty = atomic_load(&head);
		atomic_store(&pred, empty);

		empty = atomic_load(&pred->next);
		atomic_store(&curr, empty);

		curr->lock();

		while (curr->key < key) {
			pred->unlock();

			empty = atomic_load(&curr);
			atomic_store(&pred, empty);

			empty = atomic_load(&curr->next);
			atomic_store(&curr, empty);

			curr->lock();
		}

		if (key == curr->key) {
			curr->unlock();
			pred->unlock();
			return true;
		}

		else {
			curr->unlock();
			pred->unlock();
			return false;
		}
	}

	void display20() {
		int c = 20;
		std::shared_ptr<NODE> p = head->next;
		while (p != tail) {
			std::cout << p->key << ", ";
			p = p->next;
			c--;
			if (c == 0)
				return;
		}
	}
};

LFSPLIST lfsplist;

CLIST clist;

ATOMICN atomiclist;

void thread_lfsp_func(int num_thread) {
	int key;
	for (int i = 0; i < NUM_TEST / num_thread; i++) {
		switch (rand() % 3) {

		case 0:
			key = rand() % KEY_RANGE;
			lfsplist.Add(key);
			break;

		case 1:
			key = rand() % KEY_RANGE;
			lfsplist.Remove(key);
			break;

		case 2:
			key = rand() % KEY_RANGE;
			lfsplist.Contains(key);
			break;

		default:
			std::cout << "Error\n";
			exit(-1);
		}
	}
}

void thread_std_func(int num_thread) {
	int key;
	for (int i = 0; i < NUM_TEST / num_thread; i++) {
		switch (rand() % 3) {

		case 0:
			key = rand() % KEY_RANGE;
			clist.Add(key);
			break;

		case 1:
			key = rand() % KEY_RANGE;
			clist.Remove(key);
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
}

void thread_atomic_func(int num_thread) {
	int key;
	for (int i = 0; i < NUM_TEST / num_thread; i++) {
		switch (rand() % 3) {

		case 0:
			key = rand() % KEY_RANGE;
			atomiclist.Add(key);
			break;

		case 1:
			key = rand() % KEY_RANGE;
			atomiclist.Remove(key);
			break;

		case 2:
			key = rand() % KEY_RANGE;
			atomiclist.Contains(key);
			break;

		default:
			std::cout << "Error\n";
			exit(-1);
		}
	}
}

int main() {
	std::cout << "[세밀한 동기화 LFSP::shared_ptr]" << std::endl;

	for (int num_thread = 1; num_thread <= 8; num_thread *= 2) {
		lfsplist.Init();

		std::vector<std::thread> threads;

		auto start_time = high_resolution_clock::now();

		for (int i = 0; i < num_thread; ++i)
			threads.emplace_back(thread_lfsp_func, num_thread);

		for (auto &th : threads)	th.join();

		auto end_time = high_resolution_clock::now();
		auto exec_time = end_time - start_time;
		int exec_ms = duration_cast<milliseconds>(exec_time).count();

		std::cout << "Threads[" << num_thread << "] \t";
		lfsplist.display20();

		std::cout << /*"SUM : " << sum <<*/ "\t Exec_Time : " << exec_ms << "ms" << std::endl;
	}

	std::cout << "[세밀한 동기화 std::shared_ptr]" << std::endl;


	for (int num_thread = 1; num_thread <= 8; num_thread *= 2) {
		clist.Init();

		std::vector<std::thread> threads;

		auto start_time = high_resolution_clock::now();

		for (int i = 0; i < num_thread; ++i)
			threads.emplace_back(thread_std_func, num_thread);

		for (auto &th : threads)	th.join();

		auto end_time = high_resolution_clock::now();
		auto exec_time = end_time - start_time;
		int exec_ms = duration_cast<milliseconds>(exec_time).count();

		std::cout << "Threads[" << num_thread << "] \t";
		clist.display20();

		std::cout << /*"SUM : " << sum <<*/ "\t Exec_Time : " << exec_ms << "ms" << std::endl;
	}

	std::cout << "[세밀한 동기화 std::atomic shared_ptr]" << std::endl;

	for (int num_thread = 1; num_thread <= 8; num_thread *= 2) {
		atomiclist.Init();

		std::vector<std::thread> threads;

		auto start_time = high_resolution_clock::now();

		for (int i = 0; i < num_thread; ++i)
			threads.emplace_back(thread_atomic_func, num_thread);

		for (auto &th : threads)	th.join();

		auto end_time = high_resolution_clock::now();
		auto exec_time = end_time - start_time;
		int exec_ms = duration_cast<milliseconds>(exec_time).count();

		std::cout << "Threads[" << num_thread << "] \t";
		atomiclist.display20();

		std::cout << /*"SUM : " << sum <<*/ "\t Exec_Time : " << exec_ms << "ms" << std::endl;
	}

}