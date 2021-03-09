#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <vector>

#define __LOCK_FREE_SMART_POINTER__

#include "Lock-Free_Smart_Pointer.h" 

using namespace std::chrono;

const auto num_func = 1'0'000;
const auto key_range = 1000;

// Change the number of threads and the number of repeat
const auto num_of_repeat = 4; 

// Number of threads per num_of_repea
int return_thread_count(int in) { 
	if (in == 1)	return 1;
	if (in == 2)	return 2;
	if (in == 3)	return 4;
	if (in == 4)	return 8;
	if (in == 5)	return 12;
	else			return 0;
}

class NODE {
public:
	int key;
	NODE* next; bool removed;
	std::mutex nlock;

	NODE() { next = nullptr; removed = false; }
	NODE(int key_value) { next = nullptr; key = key_value; removed = false; }

	~NODE() {}

	void lock() {
		nlock.lock();
	}

	void unlock() {
		nlock.unlock();
	}
};

class ZSL {
	NODE* head;
	NODE* tail;

public:
	ZSL() {
		head = new NODE(0x80000000);
		tail = new NODE(0x7fffffff);
		head->next = tail;
	}

	~ZSL() {
		head = nullptr;
		tail = nullptr;
	}

	void Init() {
		head->next = tail;
	}

	bool validate(const NODE* pred, const NODE* curr) {
		return !pred->removed && !curr->removed && pred->next == curr;
	}

	bool Add(int key) {

		while (true) {
			NODE* pred{ head };
			NODE* curr{ pred->next };
			
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
					NODE* add_node = new NODE(key);
					add_node->next = curr;
					pred->next = add_node;

					curr->unlock();	pred->unlock();
					return true;
				}
			}
			curr->unlock();	pred->unlock();
		}
	}

	bool Remove(int key) {

		while (true) {
			NODE* pred{ head };
			NODE* curr{ pred->next };

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
		NODE* curr = head;
	
		while (curr->key < key) {
			curr = curr->next;
		}

		return curr->key == key && !(curr->removed);
	}

	void display20() {
		NODE *p = head->next;
		int c = 20;
		
		while (p != tail) {
			std::cout << p->key << ", ";
			p = p->next;
			c--;
			if (c == 0)
				break;
		}
		std::cout << std::endl;
	}
};

class SPNODE : public std::enable_shared_from_this<SPNODE> {
public:
	int key;
	std::shared_ptr<SPNODE> next; bool removed;
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

class SPZSL {
	std::shared_ptr<SPNODE> head, tail;

public:
	SPZSL() {
		head = std::make_shared<SPNODE>(0x80000000);
		tail = std::make_shared<SPNODE>(0x7fffffff);
		head->next = tail;
	}

	~SPZSL() {
		head = nullptr;
		tail = nullptr;
	}

	void Init() {
		head->next = tail;
	}

	bool validate(const std::shared_ptr<SPNODE>& pred, const std::shared_ptr<SPNODE>& curr) {
		return !pred->removed && !curr->removed && pred->next == curr;
	}

	bool Add(int key) {

		while (true) {
			std::shared_ptr<SPNODE> pred{ head };
			std::shared_ptr<SPNODE> curr{ pred->next };
		
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
					std::shared_ptr<SPNODE> add_node = std::make_shared<SPNODE>(key);
					add_node->next = curr;
					pred->next = add_node;

					curr->unlock();	pred->unlock();
					return true;
				}
			}
			curr->unlock();	pred->unlock();
		}
	}

	bool Remove(int key) {

		while (true) {
			std::shared_ptr<SPNODE> pred{ head };
			std::shared_ptr<SPNODE> curr{ pred->next };

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
		std::shared_ptr<SPNODE> curr = head;
	
		while (curr->key < key) {
			curr = curr->next;
		}

		return curr->key == key && !(curr->removed);
	}

	void display20() {
		std::shared_ptr<SPNODE>p = head->next;
		int c = 20;

		while (p.get() != tail.get()) {
			std::cout << p->key << ", ";
			p = p->next;
			c--;
			if (c == 0)
				break;
		}
		std::cout << std::endl;
	}
};

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

class ATSPZSL {
	std::shared_ptr<ATSPNODE> head, tail;

public:
	ATSPZSL() {
		head = std::make_shared<ATSPNODE>(0x80000000);
		tail = std::make_shared<ATSPNODE>(0x7fffffff);
		head->next = tail;
	}

	~ATSPZSL() {
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
			std::shared_ptr<ATSPNODE> pred{ atomic_load(&head) };
			std::shared_ptr<ATSPNODE> curr{ atomic_load(&pred->next) };
			
			while (curr->key < key) {
				pred = atomic_load(&curr);
				curr = atomic_load(&curr->next);
			}
			pred->lock();	curr->lock();

			if (validate(pred, curr)) {

				if (key == curr->key) {
					curr->unlock();	pred->unlock();
					return false;
				}

				else {
					std::shared_ptr<ATSPNODE> add_node = std::make_shared<ATSPNODE>(key);
					add_node->next = atomic_load(&curr);
					atomic_store(&pred->next, add_node);

					curr->unlock();	pred->unlock();
					return true;
				}
			}
			curr->unlock();	pred->unlock();
		}
	}

	bool Remove(int key) {

		while (true) {
			std::shared_ptr<ATSPNODE> pred{ atomic_load(&head) };
			std::shared_ptr<ATSPNODE> curr{ atomic_load(&pred->next) };

			while (curr->key < key) {
				pred = atomic_load(&curr);
				curr = atomic_load(&curr->next);
			}
			pred->lock();	curr->lock();

			if (validate(pred, curr)) {

				if (key == curr->key) {
					curr->removed = true;
					atomic_store(&pred->next, atomic_load(&curr->next));

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
		
		while (curr->key < key) {
			curr = atomic_load(&curr->next);
		}

		return curr->key == key && !(curr->removed);
	}

	void display20() {
		std::shared_ptr<ATSPNODE>p = head->next;
		int c = 20;

		while (p.get() != tail.get()) {
			std::cout << p->key << ", ";
			p = p->next;
			c--;
			if (c == 0)
				break;
		}
		std::cout << std::endl;
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
			LF::shared_ptr<LFSPNODE> pred{ head }; 
			LF::shared_ptr<LFSPNODE> curr{ pred->next };

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
			LF::shared_ptr<LFSPNODE> pred{ head };
			LF::shared_ptr<LFSPNODE> curr{ pred->next };

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

	void display20() {
		LF::shared_ptr<LFSPNODE>p = head->next;
		int c = 20;

		while (p.get() != tail.get()) {
			std::cout << p->key << ", ";
			p = p->next;
			c--;
			if (c == 0)
				break;
		}
		std::cout << std::endl;
	}
};

ZSL   zsl;
SPZSL spzsl;
ATSPZSL atzsl;
LFZSL lfzsl;

void ZSL_thread_func(int num_of_thread) {
	int key;
	for (int i = 0; i < num_func / num_of_thread; i++) {
		switch (rand() % 3) {

		case 0:
			key = rand() % key_range;
			zsl.Add(key);
			break;

		case 1:
			key = rand() % key_range;
			zsl.Remove(key);
			break;

		case 2:
			key = rand() % key_range;
			zsl.Contains(key);
			break;

		default:
			std::cout << "Error\n";
			exit(-1);
		}
	}
}

void SPZSL_thread_func(int num_of_thread) {
	int key;
	for (int i = 0; i < num_func / num_of_thread; i++) {
		switch (rand() % 3) {

		case 0:
			key = rand() % key_range;
			spzsl.Add(key);
			break;

		case 1:
			key = rand() % key_range;
			spzsl.Remove(key);
			break;

		case 2:
			key = rand() % key_range;
			spzsl.Contains(key);
			break;

		default:
			std::cout << "Error\n";
			exit(-1);
		}
	}
}

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
	std::cout << "\t\tBenchmark Program\n" << std::endl;
	std::cout << "\tNUM_TEST = " << num_func << "\tKEY_RANGE = " << key_range << "\n";
	std::cout << "\t    opms = operation/millisecond sec\n";
	std::cout << "-------------------------------------------------------" << std::endl;

	std::cout << "\n\t\t< ZSL >" << std::endl;
	for (int count_of_repeat = 1; count_of_repeat <= num_of_repeat; count_of_repeat++) {

		int num_of_thread = return_thread_count(count_of_repeat);
		std::cout << std::endl << "[thread " << num_of_thread << " ]\t";

		zsl.Init();

		std::vector<std::thread> threads;

		auto start_time = high_resolution_clock::now();

		for (int i = 0; i < num_of_thread; ++i)
			threads.emplace_back(ZSL_thread_func, num_of_thread);

		for (auto &th : threads)	th.join();

		auto end_time = high_resolution_clock::now();
		auto exec_time = end_time - start_time;
		long long exec_ms = duration_cast<milliseconds>(exec_time).count();

		float opms = (float)num_func / (float)exec_ms;

		zsl.display20();
		std::cout << exec_ms << "ms\t\t" << "(opms : " << opms << " )";
	}

	std::cout << "\n\n\t\t< SPZSL >" << std::endl;
	for (int count_of_repeat = 1; count_of_repeat <= 1; count_of_repeat++) {

		int num_of_thread = return_thread_count(count_of_repeat);
		std::cout << std::endl << "[thread " << num_of_thread << " ]\t";

		spzsl.Init();

		std::vector<std::thread> threads;

		auto start_time = high_resolution_clock::now();

		for (int i = 0; i < num_of_thread; ++i)
			threads.emplace_back(SPZSL_thread_func, num_of_thread);

		for (auto &th : threads)	th.join();

		auto end_time = high_resolution_clock::now();
		auto exec_time = end_time - start_time;
		long long exec_ms = duration_cast<milliseconds>(exec_time).count();

		float opms = (float)num_func / (float)exec_ms;

		spzsl.display20();
		std::cout << exec_ms << "ms\t\t" << "(opms : " << opms << " )";
	}

	std::cout << "\n\n\t\t< ATSPZSL >" << std::endl;
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

		atzsl.display20();
		std::cout << exec_ms << "ms\t\t" << "(opms : " << opms << " )";
	}

	std::cout << "\n\n\t\t< LFZSL >" << std::endl;
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

		lfzsl.display20();
		std::cout << exec_ms << "ms\t\t" << "(opms : " << opms << " )";
	}

	std::cout << "\n\n";

	int key_input;
	std::cin >> key_input;
}