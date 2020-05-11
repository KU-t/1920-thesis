#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>
#include "blocking_shared_ptr.h"

using namespace LFSP;
using namespace std::chrono;

const auto NUM_TEST = 200;
const auto KEY_RANGE = 1000;

class SPNODE {
public:
	int key;
	shared_ptr<SPNODE> next; bool removed;
	std::mutex nlock;

	SPNODE() { next = nullptr; removed = false; }
	SPNODE(int key_value) { next = nullptr; key = key_value; removed = false; }

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
		head->next = tail;
	}

	~SPCLIST() {
		head = nullptr;
		tail = nullptr;
	}

	void Init() {
		head->next = tail;
	}

	bool validate( shared_ptr<SPNODE>& pred,  shared_ptr<SPNODE>& curr) {
		return !pred->removed && !curr->removed && pred->next == curr;
	}

	bool Add(int key) {
		while (true) {
			shared_ptr<SPNODE> pred, curr;
			shared_ptr<SPNODE> empty = atomic_load(&head);
			atomic_store(&pred, empty);

			empty = atomic_load(&pred->next);
			atomic_store(&curr, empty);

			while (curr->key < key) {
				empty = atomic_load(&curr);
				atomic_store(&pred, empty);

				empty = atomic_load(&curr->next);
				atomic_store(&curr, empty);
			}
			pred->lock();	curr->lock();

			if (validate(pred, curr)) {
				if (key == curr->key) {
					curr->unlock();	pred->unlock();
					return false;
				}

				else {
					shared_ptr<SPNODE> add_node = make_shared<SPNODE>(key);

					empty = atomic_load(&curr);
					add_node->next = empty;

					empty = atomic_load(&add_node);
					atomic_store(&pred->next, empty);

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

			shared_ptr<SPNODE> empty = atomic_load(&head);
			atomic_store(&pred, empty);

			empty = atomic_load(&pred->next);
			atomic_store(&curr, empty);

			while (curr->key < key) {
				empty = atomic_load(&curr);
				atomic_store(&pred, empty);

				empty = atomic_load(&curr->next);
				atomic_store(&curr, empty);
			}
			pred->lock();	curr->lock();

			if (validate(pred, curr)) {
				if (key == curr->key) {
					empty->removed = true;
					atomic_store(&curr, empty);

					empty = atomic_load(&curr->next);
					atomic_store(&pred->next, empty);

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
		shared_ptr<SPNODE> curr = atomic_load(&head);
		shared_ptr<SPNODE> empty;

		while (curr->key < key) {

			empty = atomic_load(&curr->next);
			atomic_store(&curr, empty);
		}

		return empty->key == key && !empty->removed;
	}

	void display20() {
		int c = 20;
		shared_ptr<SPNODE> p = head->next;
		while (p != tail) {
			std::cout << p->key << ", ";
			p = p->next;
			c--;
			if (c == 0)
				break;
		}
	}
};

SPCLIST clist;

void thread_func(int num_thread) {
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

int main() {

	for (int num_thread = 1; num_thread <= 8; num_thread *= 2) {
		clist.Init();

		std::vector<std::thread> threads;

		auto start_time = high_resolution_clock::now();

		for (int i = 0; i < num_thread; ++i)
			threads.emplace_back(thread_func, num_thread);

		for (auto &th : threads)	th.join();

		auto end_time = high_resolution_clock::now();
		auto exec_time = end_time - start_time;
		int exec_ms = duration_cast<milliseconds>(exec_time).count();

		std::cout << "Threads[" << num_thread << "] \t";
		clist.display20();

		std::cout << "\t Exec_Time : " << exec_ms << "ms" << std::endl;
	}
}