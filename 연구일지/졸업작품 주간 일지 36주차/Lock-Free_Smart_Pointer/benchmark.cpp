#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <vector>

#include "Lock-Free_Smart_Pointer.h" 

using namespace std::chrono;

const auto num_func = 1'200'000;
const auto key_range = 100'000;

// Change the number of threads and the number of repeat
const auto num_of_repeat = 5; 

// Number of threads per num_of_repea
int return_thread_count(int in) { 
	if (in == 1)	return 1;
	if (in == 2)	return 2;
	if (in == 3)	return 4;
	if (in == 4)	return 6;
	if (in == 5)	return 12;
	else			return 0;
}

class ATSPNODE : public std::enable_shared_from_this<ATSPNODE> {
public:
	int key;
	std::shared_ptr<ATSPNODE> next; bool removed;
	std::mutex nlock;

	ATSPNODE() { next = nullptr; removed = false; }
	ATSPNODE(int key_value) { next = nullptr; key = key_value; removed = false; }

	~ATSPNODE() {}

	void lock() {
		nlock.lock();
	}

	void unlock() {
		nlock.unlock();
	}
};

class ATZSL {
	std::shared_ptr<ATSPNODE> head, tail;

public:
	ATZSL() {
		head = std::make_shared<ATSPNODE>(0x80000000);
		tail = std::make_shared<ATSPNODE>(0x7fffffff);
		head->next = tail;
	}

	~ATZSL() {
		head = nullptr;
		tail = nullptr;
	}

	void Init() {
		head->next = tail;
	}

	bool validate(const std::shared_ptr<ATSPNODE>& pred, const std::shared_ptr<ATSPNODE>& curr) {
		return !pred->removed && !curr->removed && pred->next == curr;
	}

	bool Add(int key) {
		while (true) {
			std::shared_ptr<ATSPNODE> pred, curr;
			std::shared_ptr<ATSPNODE> empty = atomic_load(&head);
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
					std::shared_ptr<ATSPNODE> add_node = std::make_shared<ATSPNODE>(key);

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
			std::shared_ptr<ATSPNODE> pred, curr;

			std::shared_ptr<ATSPNODE> empty = atomic_load(&head);
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
		std::shared_ptr<ATSPNODE> curr = atomic_load(&head);
		std::shared_ptr<ATSPNODE> empty;

		while (curr->key < key) {

			empty = atomic_load(&curr->next);
			atomic_store(&curr, empty);
		}

		return empty->key == key && !empty->removed;
	}
};

class LFSPNODE : public LF::enable_shared_from_this<LFSPNODE> {
public:
	int key;
	LF::shared_ptr<LFSPNODE> next; bool removed;
	std::mutex nlock;

	LFSPNODE() { next = nullptr; removed = false; }
	LFSPNODE(int key_value) { next = nullptr; key = key_value; removed = false; }

	~LFSPNODE() {}

	void lock() {
		nlock.lock();
	}

	void unlock() {
		nlock.unlock();
	}
};

class LFZSL {
	LF::shared_ptr<LFSPNODE> head, tail;

public:
	LFZSL() {
		head = LF::make_shared<LFSPNODE>(0x80000000);
		tail = LF::make_shared<LFSPNODE>(0x7fffffff);
		head->next = tail;

	}

	~LFZSL() {
		head = nullptr;
		tail = nullptr;
	}

	void Init() {
		head->next = tail;
	}

	bool validate(const LF::shared_ptr<LFSPNODE>& pred, const LF::shared_ptr<LFSPNODE>& curr) {
		return !pred->removed && !curr->removed && pred->next == curr;
	}

	bool Add(int key) {

		while (true) {

			LF::shared_ptr<LFSPNODE> pred, curr;
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
					LF::shared_ptr<LFSPNODE> node = LF::make_shared<LFSPNODE>(key);

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
			LF::shared_ptr<LFSPNODE> pred, curr;
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
		LF::shared_ptr<LFSPNODE> curr = head;
		while (curr->key < key) {
			curr = curr->next;
		}
		return curr->key == key && !curr->removed;
	}
};

ATZSL atzsl;
LFZSL lfzsl;

void ATZSL_thread_func(int num_of_thread) {
	int key;
	for (int i = 0; i < num_func / num_of_thread; i++) {
		switch (rand() % 3) {

		case 0:
			key = rand() % key_range;
			atzsl.Add(key);
			break;

		case 1:
			key = rand() % key_range;
			atzsl.Remove(key);
			break;

		case 2:
			key = rand() % key_range;
			atzsl.Contains(key);
			break;

		default:
			std::cout << "Error\n";
			exit(-1);
		}
	}
}

void LFZSL_thread_func(int num_of_thread) {
	int key;
	for (int i = 0; i < num_func / num_of_thread; i++) {
		switch (rand() % 3) {

		case 0:
			key = rand() % key_range;
			lfzsl.Add(key);
			break;

		case 1:
			key = rand() % key_range;
			lfzsl.Remove(key);
			break;

		case 2:
			key = rand() % key_range;
			lfzsl.Contains(key);
			break;

		default:
			std::cout << "Error\n";
			exit(-1);
		}
	}
}

int main() {

	std::cout << "-------------------------------------------------------" << std::endl;
	std::cout << "\tImplementation of Lock-Free Smart Pointer" << std::endl;
	std::cout << "\t    for multi-threaded environments\n" << std::endl;
	std::cout << "\t\t[benchmark program]\n" << std::endl;
	std::cout << "\tNUM_TEST = " << num_func << "\tKEY_RANGE = " << key_range << "\n";
	std::cout << "\t    opms = operation/millisecond sec\n";
	std::cout << "-------------------------------------------------------" << std::endl;

	std::cout << "\n\t\t< ATZSL >" << std::endl;
	for (int count_of_repeat = 1; count_of_repeat <= num_of_repeat; count_of_repeat++) {
		
		int num_of_thread = return_thread_count(count_of_repeat);
		std::cout << std::endl << "[thread " << num_of_thread << " ]\t";

		atzsl.Init();

		std::vector<std::thread> threads;

		auto start_time = high_resolution_clock::now();

		for (int i = 0; i < num_of_thread; ++i)
			threads.emplace_back(ATZSL_thread_func, num_of_thread);

		for (auto &th : threads)	th.join();

		auto end_time = high_resolution_clock::now();
		auto exec_time = end_time - start_time;
		long long exec_ms = duration_cast<milliseconds>(exec_time).count();

		float opms = (float)num_func / (float)exec_ms;

		std::cout << exec_ms << "ms\t\t" << "(opms : " << opms << " )";
	}

	std::cout << "\n\n\n\t\t< LFZSL >" << std::endl;
	for (int count_of_repeat = 1; count_of_repeat <= num_of_repeat; count_of_repeat++) {
		
		int num_of_thread = return_thread_count(count_of_repeat); 
		std::cout << std::endl << "[thread " << num_of_thread << " ]\t";

		lfzsl.Init();

		std::vector<std::thread> threads;

		auto start_time = high_resolution_clock::now();

		for (int i = 0; i < num_of_thread; ++i)
			threads.emplace_back(LFZSL_thread_func, num_of_thread);

		for (auto &th : threads)	th.join();

		auto end_time = high_resolution_clock::now();
		auto exec_time = end_time - start_time;
		long long exec_ms = duration_cast<milliseconds>(exec_time).count();

		float opms = (float)num_func / (float)exec_ms;

		std::cout << exec_ms << "ms\t\t" << "(opms : " << opms << " )";
	}

	std::cout << "\n\n";
}