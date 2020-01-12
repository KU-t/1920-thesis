#pragma once
#include "control_block.h"
#include <type_traits>

template<typename T> 
class shared_ptr {
private:
	//typedef T element_type;
	//using element_type = remove_extent_t<T>;

	T* ptr;
	control_block* c_block;

public:		// ����, �̵�, �Ҹ�
	constexpr shared_ptr(T* p = nullptr) noexcept;
	shared_ptr(const shared_ptr<T>& other) noexcept;
	shared_ptr(shared_ptr<T>&& other) noexcept;
	/*
	template <class Other>
	explicit shared_ptr(Other* ptr);

	template <class Other, class Deleter>
	shared_ptr(Other* ptr, Deleter deleter);

	template <class Deleter>
	shared_ptr(nullptr_t ptr, Deleter deleter);

	template <class Other, class Deleter, class Allocator>
	shared_ptr(Other* ptr, Deleter deleter, Allocator alloc);

	template <class Deleter, class Allocator>
	shared_ptr(nullptr_t ptr, Deleter deleter, Allocator alloc);

	template <class Other>
	shared_ptr(const shared_ptr<Other>& sp) noexcept;

	template <class Other>
	explicit shared_ptr(const weak_ptr<Other>& wp);

	template <class &>
	shared_ptr(std::auto_ptr<Other>& ap);

	template <class &>
	shared_ptr(std::auto_ptr<Other>&& ap);

	template <class Other, class Deleter>
	shared_ptr(unique_ptr<Other, Deleter>&& up);

	template <class Other>
	shared_ptr(const shared_ptr<Other>& sp, element_type* ptr) noexcept;

	template <class Other>
	shared_ptr(shared_ptr<Other>&& sp, element_type* ptr) noexcept;

	template <class Other, class Deleter>
	shared_ptr(const unique_ptr<Other, Deleter>& up) = delete;
	*/

	~shared_ptr();
public:
	template<typename Other>
	Other* Getptr() const noexcept { return ptr; }

public:		// ������
	T& operator*() const noexcept;
	T* operator->() const noexcept;
	bool operatorbool() const noexcept;

	// ���ҽ� ����

	// ���� Ÿ��
	shared_ptr<T>& operator=(const shared_ptr<T>& other) noexcept;
	shared_ptr<T>& operator=(shared_ptr<T>&& other) noexcept;

	// �ٸ� Ÿ��
	// ���� [in] <Other> [out] <T> ??
	template<typename Other>
	shared_ptr<T>& operator=(const shared_ptr<Other>& other) noexcept {
		T* other_type = reinterpret_cast<T>(&(other->ptr));
		//shared_ptr<T>* empty = reinterpret_cast<shared_ptr<T>>(&other);
				
		/*if (this == &other)	return *this;

		if (c_block != nullptr) {
			if (c_block->decre_s_ref() == 0) {
				delete ptr;
				delete c_block;
			}
		}
		ptr = other.ptr;
		c_block = other.c_block;
		c_block->incre_s_ref();*/
		return *this;
	}

	// �̵� [in] <Other> [out] <T> ??
	template <typename Other>
	shared_ptr<T>& operator=(shared_ptr<Other>&& other) noexcept{
		/*if (this == &other)	return *this;
		if (c_block->decre_s_ref() == 0) {
			delete ptr;
			delete c_block;
		}
		ptr = other.ptr;
		c_block = other.ptr;

		other.ptr = nullptr;
		other.c_block = nullptr;*/
		return *this;
	}

	/*
	// unique_ptr ���� [in] <Other> [out] <T>
	template <class Other, class Deleter>
	shared_ptr& operator=(unique_ptr<Other, Deleter>&& up) {

	}
	*/


public:		// �޼ҵ�
	T* get() const noexcept;
	void swap(shared_ptr<T>& other) noexcept;
	int use_count() const;
	
	// ���ҽ� ����
	void reset();

	/*
	template <class Other>
	void reset(Other *ptr);

	template <class Other, class Deleter>
	void reset(Other *ptr, Deleter deleter);

	template <class Other, class Deleter, class Allocator>
	void reset(Other *ptr, Deleter deleter, Allocator alloc);
	*/

	
	// ����	??
	/*
	template <class Other>
	bool owner_before(const shared_ptr<Other>& ptr) const noexcept;

	template <class Other>
	bool owner_before(const weak_ptr<Other>& ptr) const noexcept;
	*/
};

// -----------------------------------------------------

// ������ 
template<typename T>
constexpr shared_ptr<T>::shared_ptr(T* p) noexcept{
	ptr = p;
	c_block = new control_block();

	if (ptr != nullptr) c_block->incre_s_ref();
}

// �������
template<typename T>
shared_ptr<T>::shared_ptr(const shared_ptr<T>& other) noexcept {
	ptr = other.ptr;
	c_block = other.c_block;
	c_block->incre_s_ref();
}

// �̵�����
template<typename T>
shared_ptr<T>::shared_ptr(shared_ptr<T>&& other) noexcept {
	ptr = other.ptr;
	c_block = other.ptr;

	other.ptr = nullptr;
	other.c_block = nullptr;
}

// �Ҹ��� 
template<typename T>
shared_ptr<T>::~shared_ptr() {
	if (ptr == nullptr)	return;
	if (c_block->decre_s_ref() == 0) {
		delete ptr;
		delete c_block;
	}
}

// -------------- [������] ---------------

// * : ������ �� ����
template<typename T>
T& shared_ptr<T>::operator*() const noexcept {
	return *ptr;
}

// -> : ������ ������ ������ ����
template<typename T>
T* shared_ptr<T>::operator->() const noexcept {
	return ptr; 
}

// bool : ���ҽ� ���� ���� ����
template<typename T>
bool shared_ptr<T>::operatorbool() const noexcept {
	if (ptr != nullptr)	return true;
	return false;
}

// -------------- [operator] ---------------

// ����
template<typename T>
shared_ptr<T>& shared_ptr<T>::operator=(const shared_ptr<T>& other) noexcept {
	if (this == &other)	return *this;
	if (c_block != nullptr) {
		if (c_block->decre_s_ref() == 0) {
			delete ptr;
			delete c_block;
		}
	}
	ptr = other.ptr;
	c_block = other.c_block;
	c_block->incre_s_ref();
}

// �̵��Ҵ�
template<typename T>
shared_ptr<T>& shared_ptr<T>::operator=(shared_ptr<T>&& other) noexcept {
	if (this == &other)	return *this;
	if (c_block->decre_s_ref() == 0) {
		delete ptr;
		delete c_block;
	}
	ptr = other.ptr;
	c_block = other.ptr;

	other.ptr = nullptr;
	other.c_block = nullptr;
}

//template<typename T, typename Other>
//shared_ptr<T>& shared_ptr<T>::operator=(const shared_ptr<Other>& sp) noexcept {
//
//}

// -------------- [�޼ҵ�] ---------------

// get
template<typename T>
T* shared_ptr<T>::get() const noexcept{
	if (ptr == nullptr)	return 0;
	return ptr;
}

// swap
template<typename T>
void shared_ptr<T>::swap(shared_ptr<T>& other) noexcept {
	T* emptyT = other.ptr;
	control_block* emptyCB = other.c_block;

	other.ptr = ptr;
	other.c_block = c_block;

	ptr = emptyT;
	c_block = emptyCB;
}

// use_count
template<typename T>
inline int shared_ptr<T>::use_count() const {
	if (c_block == nullptr)	return 0;
	return c_block->Getstrongref();
}

// reset
template<typename T>
void shared_ptr<T>::reset() {
	ptr = nullptr;
	if (c_block->decre_s_ref() == 0) {
		delete ptr;
		delete c_block;
	}
	c_block = nullptr;
}

// ------------------------------------------------------------------

template <typename T>
T* make_shared(int n = 0) {
	return new T(n);
}

