#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>

using namespace std;
using namespace std::chrono;

const auto NUM_TEST = 10000;
const auto KEY_RANGE = 1000;

class NODE {
public:
	int key;
	shared_ptr<NODE> next;
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
	shared_ptr<NODE> head, tail;
public:
	CLIST() {
		head = make_shared<NODE>(0x80000000);
		tail = make_shared<NODE>(0x7fffffff);
		head->next = tail;
	}

	~CLIST() {}

	void Init() {
		std::cout << "Clist.Init()" << std::endl;
		head->next = tail;
	}

	bool Add(int key) {

		shared_ptr<NODE> pred, curr;
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
			shared_ptr<NODE> add_node = make_shared<NODE>(key);
			add_node->next = curr;
			pred->next = add_node;
			curr->unlock();
			pred->unlock();
			return true;
		}
	}

	bool Remove(int key) {
		shared_ptr<NODE> pred, curr;
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
		shared_ptr<NODE> pred, curr;
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
		shared_ptr<NODE> p = head->next;
		while (p != tail) {
			std::cout << p->key << ", ";
			p = p->next;
			c--;
			if (c == 0)
				return;
		}
	}
};

CLIST clist;

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
	std::cout << "[세밀한 동기화 std::shared_ptr]" << std::endl;

	
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

			std::cout << /*"SUM : " << sum <<*/ "\t Exec_Time : " << exec_ms << "ms" << std::endl;
		}
}