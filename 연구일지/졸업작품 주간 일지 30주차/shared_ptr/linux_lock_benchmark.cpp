#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>

#include "linux_blocking_shared_ptr.h"

using namespace std::chrono;

const auto NUM_TEST = 300000;
const auto KEY_RANGE = 10000;

const auto Try_test = 100;
const auto number_of_threads = 64;

class LSPNODE : public LSP::enable_shared_from_this<LSPNODE> {
public:
	int key;
	LSP::shared_ptr<LSPNODE> next; bool removed;
	std::mutex nlock;

	LSPNODE() { next = nullptr; removed = false; }
	LSPNODE(int key_value) { next = nullptr; key = key_value; removed = false; }

	~LSPNODE() {}

	void lock() {
		nlock.lock();
	}

	void unlock() {
		nlock.unlock();
	}
};

class LSPCLIST {
	LSP::shared_ptr<LSPNODE> head, tail;

public:
	LSPCLIST() {
		head = LSP::make_shared<LSPNODE>(0x80000000);
		tail = LSP::make_shared<LSPNODE>(0x7fffffff);
		head->next = tail;

	}

	~LSPCLIST() {
		head = nullptr;
		tail = nullptr;
	}

	void Init() {
		head->next = tail;
	}

	bool validate(const LSP::shared_ptr<LSPNODE>& pred, const LSP::shared_ptr<LSPNODE>& curr) {
		return !pred->removed && !curr->removed && pred->next == curr;
	}

	bool Add(int key) {

		while (true) {

			LSP::shared_ptr<LSPNODE> pred, curr;
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
					LSP::shared_ptr<LSPNODE> node = LSP::make_shared<LSPNODE>(key);

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
			LSP::shared_ptr<LSPNODE> pred, curr;
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
		LSP::shared_ptr<LSPNODE> curr = head;
		while (curr->key < key) {
			curr = curr->next;
		}
		return curr->key == key && !curr->removed;
	}

	void display20() {
		int c = 20;
		LSP::shared_ptr<LSPNODE> p = head->next;
		while (p != tail) {
			std::cout << p->key << ", ";
			p = p->next;
			c--;
			if (c == 0)
				return;
		}
	}
};

LSPCLIST lsplist;

void lsp_thread_func(int num_thread) {
	int key;
	for (int i = 0; i < NUM_TEST / num_thread; i++) {
		switch (rand() % 3) {

		case 0:
			key = rand() % KEY_RANGE;
			lsplist.Add(key);
			break;

		case 1:
			key = rand() % KEY_RANGE;
			lsplist.Remove(key);
			break;

		case 2:
			key = rand() % KEY_RANGE;
			lsplist.Contains(key);
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
			lsplist.Init();

			std::vector<std::thread> threads;

			auto start_time = high_resolution_clock::now();

			for (int i = 0; i < num_thread; ++i)
				threads.emplace_back(lsp_thread_func, num_thread);

			for (auto &th : threads)	th.join();

			auto end_time = high_resolution_clock::now();
			auto exec_time = end_time - start_time;
			long long exec_ms = duration_cast<milliseconds>(exec_time).count();

			std::cout << "Threads[" << num_thread << "] \t";
			lsplist.display20();

			std::cout << "\t Exec_Time : " << exec_ms << "ms" << std::endl;
		}
	}
}