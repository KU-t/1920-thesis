#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>

#include "linux_lockfree_shared_ptr.h"

using namespace std::chrono;

const auto NUM_TEST = 100000;
const auto KEY_RANGE = 10000;

const auto Try_test = 100;
const auto number_of_threads = 8;

class SPNODE /* : public LFSP::enable_shared_from_this<SPNODE> */{
public:
	int key;
	LFSP::shared_ptr<SPNODE> next; bool removed;
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
	LFSP::shared_ptr<SPNODE> head, tail;

public:
	SPCLIST() {
		head = LFSP::make_shared<SPNODE>(0x80000000);
		tail = LFSP::make_shared<SPNODE>(0x7fffffff);
		head->next = tail;

	}

	~SPCLIST() {
		head = nullptr;
		tail = nullptr;
	}

	void Init() {
		head->next = tail;
	}

	bool validate(const LFSP::shared_ptr<SPNODE>& pred, const LFSP::shared_ptr<SPNODE>& curr) {
		return !pred->removed && !curr->removed && pred->next == curr;
	}

	bool Add(int key) {

		while (true) {

			LFSP::shared_ptr<SPNODE> pred, curr;
			pred = head;
			curr = pred->next;

			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}
			pred->lock();	curr->lock();

			if (validate(pred, curr)) {
				if (key == curr->key) {
					curr->unlock();	pred->unlock();

					return false;
				}

				else {
					LFSP::shared_ptr<SPNODE> node = LFSP::make_shared<SPNODE>(key);

					node->next = curr;
					pred->next = node;

					curr->unlock();	pred->unlock();

					return true;
				}
			}
			curr->unlock();	pred->unlock();
		}
	}

	bool Remove(int key) {

		while (true) {
			LFSP::shared_ptr<SPNODE> pred, curr;
			pred = head;
			curr = pred->next;

			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}
			pred->lock();	curr->lock();

			if (validate(pred, curr)) {
				if (key == curr->key) {
					curr->removed = true;
					pred->next = curr->next;

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
		LFSP::shared_ptr<SPNODE> curr = head;
		while (curr->key < key) {
			curr = curr->next;
		}
		return curr->key == key && !curr->removed;
	}

	void display20() {
		int c = 20;
		LFSP::shared_ptr<SPNODE> p = head->next;
		while (p != tail) {
			std::cout << p->key << ", ";
			p = p->next;
			c--;
			if (c == 0)
				return;
		}
	}
};

SPCLIST splist;

void sp_thread_func(int num_thread) {
	int key;
	for (int i = 0; i < NUM_TEST / num_thread; i++) {
		switch (rand() % 3) {

		case 0:
			key = rand() % KEY_RANGE;
			splist.Add(key);
			break;

		case 1:
			key = rand() % KEY_RANGE;
			splist.Remove(key);
			break;

		case 2:
			key = rand() % KEY_RANGE;
			splist.Contains(key);
			break;

		default:
			std::cout << "Error\n";
			exit(-1);
		}
	}
}

int main() {

	std::cout << "\nLFSP::shared_ptr(lock-free)" << std::endl;

	std::cout << "number of test : " << Try_test << " (average time)\n\n";

	std::cout << "NUM_TEST = " << NUM_TEST << "\n";
	std::cout << "KEY_RANGE = " << KEY_RANGE << "\n";

	int test = 0;

	while (++test < Try_test) {
		
		std::cout << "# " << test << "\n";

		for (int num_thread = 1; num_thread <= number_of_threads; num_thread *= 2) {
			splist.Init();

			std::vector<std::thread> threads;

			auto start_time = high_resolution_clock::now();

			for (int i = 0; i < num_thread; ++i)
				threads.emplace_back(sp_thread_func, num_thread);

			for (auto &th : threads)	th.join();

			auto end_time = high_resolution_clock::now();
			auto exec_time = end_time - start_time;
			long long exec_ms = duration_cast<milliseconds>(exec_time).count();

			std::cout << "Threads[" << num_thread << "] \t";
			splist.display20();

			std::cout << "\t Exec_Time : " << exec_ms << "ms" << std::endl;
		}
	}
}