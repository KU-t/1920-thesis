#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>
#include "blocking_shared_ptr.h"

using namespace std::chrono;

const auto NUM_TEST = 100000;
const auto KEY_RANGE = 1000;

const auto Try_test = 5;

int std_time[4][Try_test];
int lfsp_time[4][Try_test];

class STDNODE {
public:
	int key;
	std::shared_ptr<STDNODE> next; bool removed;
	std::mutex nlock;

	STDNODE() { next = nullptr; removed = false; }
	STDNODE(int key_value) { next = nullptr; key = key_value; removed = false; }

	~STDNODE() {}

	void lock() {
		nlock.lock();
	}

	void unlock() {
		nlock.unlock();
	}
};

class STDLIST {
	std::shared_ptr<STDNODE> head, tail;

public:
	STDLIST() {
		head = std::make_shared<STDNODE>(0x80000000);
		tail = std::make_shared<STDNODE>(0x7fffffff);
		head->next = tail;
	}

	~STDLIST() {
		head = nullptr;
		tail = nullptr;
	}

	void Init() {
		head->next = tail;
	}

	bool validate(const std::shared_ptr<STDNODE>& pred, const std::shared_ptr<STDNODE>& curr) {
		return !pred->removed && !curr->removed && pred->next == curr;
	}

	bool Add(int key) {
		while (true) {
			std::shared_ptr<STDNODE> pred, curr;
			std::shared_ptr<STDNODE> empty = atomic_load(&head);
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
					std::shared_ptr<STDNODE> add_node = std::make_shared<STDNODE>(key);

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
			std::shared_ptr<STDNODE> pred, curr;

			std::shared_ptr<STDNODE> empty = atomic_load(&head);
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
		std::shared_ptr<STDNODE> curr = atomic_load(&head);
		std::shared_ptr<STDNODE> empty;

		while (curr->key < key) {

			empty = atomic_load(&curr->next);
			atomic_store(&curr, empty);
		}

		return empty->key == key && !empty->removed;
	}

	void display20() {
		int c = 20;
		std::shared_ptr<STDNODE> p = head->next;
		while (p != tail) {
			std::cout << p->key << ", ";
			p = p->next;
			c--;
			if (c == 0)
				break;
		}
	}
};

class SPNODE {
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

STDLIST stdlist;
SPCLIST splist;


void std_thread_func(int num_thread) {
	int key;
	for (int i = 0; i < NUM_TEST / num_thread; i++) {
		switch (rand() % 3) {

		case 0:
			key = rand() % KEY_RANGE;
			stdlist.Add(key);
			break;

		case 1:
			key = rand() % KEY_RANGE;
			stdlist.Remove(key);
			break;

		case 2:
			key = rand() % KEY_RANGE;
			stdlist.Contains(key);
			break;

		default:
			std::cout << "Error\n";
			exit(-1);
		}
	}
}

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

	int thread_num = 0;

	std::cout << "std::shared_ptr (use atomic_load & _store)" << std::endl;

	for (int num_thread = 1; num_thread <= 8; num_thread *= 2) {
		stdlist.Init();

		for (int Test_try = 0; Test_try < Try_test; ++Test_try) {
			std::vector<std::thread> threads;

			auto start_time = high_resolution_clock::now();

			for (int i = 0; i < num_thread; ++i)
				threads.emplace_back(std_thread_func, num_thread);

			for (auto &th : threads)	th.join();

			auto end_time = high_resolution_clock::now();
			auto exec_time = end_time - start_time;
			int exec_ms = duration_cast<milliseconds>(exec_time).count();

			std::cout << "Threads[" << num_thread << "] \t";
			stdlist.display20();

			std::cout << "\t Exec_Time : " << exec_ms << "ms" << std::endl;
			std_time[thread_num][Test_try] = exec_ms;
		}
		thread_num++;
	}

	thread_num = 0;

	std::cout << "LFSP::shared_ptr" << std::endl;

	for (int num_thread = 1; num_thread <= 8; num_thread *= 2) {
		splist.Init();

		for (int Test_try = 0; Test_try < Try_test; ++Test_try) {
			std::vector<std::thread> threads;

			auto start_time = high_resolution_clock::now();

			for (int i = 0; i < num_thread; ++i)
				threads.emplace_back(sp_thread_func, num_thread);

			for (auto &th : threads)	th.join();

			auto end_time = high_resolution_clock::now();
			auto exec_time = end_time - start_time;
			int exec_ms = duration_cast<milliseconds>(exec_time).count();

			std::cout << "Threads[" << num_thread << "] \t";
			splist.display20();

			std::cout << "\t Exec_Time : " << exec_ms << "ms" << std::endl;
			lfsp_time[thread_num][Test_try] = exec_ms;
		}
		thread_num++;
	}

	std::cout << "\n[Result] - try " << Try_test<< " average time" << std::endl;
	std::cout << "NUM_TEST = " << NUM_TEST << std::endl;
	std::cout << "\t\t\t\t Thread1\t Thread2\t Thread4\t Thread8" << std::endl;
	int sum;
	
	std::cout << "std::shared_ptr :";
	for (int j = 0; j < 4; ++j) {
		sum = 0;
		for (int i = 0; i < Try_test; ++i) {
			sum += std_time[j][i];
		}
		std::cout << "\t\t " << sum;
	}

	std::cout << std::endl;
	
	std::cout << "LFSP::shared_ptr :";
	for (int j = 0; j < 4; ++j) {
		sum = 0;
		for (int i = 0; i < Try_test; ++i) {
			sum += lfsp_time[j][i];
		}
		std::cout << "\t\t " << sum ;
	}
}