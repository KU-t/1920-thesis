#pragma once

#define TEST_WEAK_PTR

#include <atomic>
#include <mutex>

// Hazard pointer
#include <windows.h>
#include <vector>
#include <algorithm>

namespace HPLFSP {

	class HpRec
	{
	public:
		HpRec* pNext;

		int active;
		PVOID pNode;

	public:
		HpRec()
		{
			this->pNext = NULL;
			this->active = 1;
			this->pNode = NULL;
		}

		~HpRec()
		{
		}

		static HpRec* Alloc(HpRec** pHead)
		{
			HpRec* p;

			//기존에 빈레코드가 있는지 찾아본다.
			for (p = *pHead; p != NULL; p = p->pNext)
			{
				if (p->active == 0 && InterlockedCompareExchange((LONG*)&(p->active), (LONG)1, (LONG)0) == 0)
					return p;
			}

			//새레코드를 하나 생성한다.
			p = new HpRec();

			//새로 생성한 레코드를 Head에 넣는다.
			HpRec* oldHead;
			do
			{
				oldHead = *pHead;
				p->pNext = oldHead;
			} while (InterlockedCompareExchangePointer((PVOID*)pHead, p, oldHead) != oldHead);

			return p;
		}

		static void Release(HpRec* p)
		{
			p->active = 0;
			p->pNode = NULL;
		}

		static void DeleteAll(HpRec** pHead)
		{
			HpRec* p = *pHead;
			HpRec* n;
			while (p != NULL)
			{
				n = p->pNext;
				delete p;
				p = n;
			}
		}
	};

	class SimpleList
	{
	public:
		class Node
		{
		public:
			Node* pPrev;
			Node* pNext;
			PVOID pData;

		public:
			Node(PVOID pData)
			{
				this->pPrev = NULL;
				this->pNext = NULL;
				this->pData = pData;
			}

			~Node()
			{
			}
		};

	public:
		Node* pHead;
		Node* pTail;
		int size;

	public:
		SimpleList()
		{
			this->pHead = NULL;
			this->pTail = NULL;
			this->size = 0;
		}

		~SimpleList()
		{
			Node* p = this->pHead;
			Node* n;
			while (p != NULL)
			{
				n = p->pNext;
				delete p->pData;
				delete p;
				p = n;
			}
		}

		Node* Append(PVOID pData)
		{
			Node* node = new Node(pData);

			if (this->pTail == NULL)
			{
				this->pHead = this->pTail = node;
			}
			else
			{
				this->pTail->pNext = node;
				node->pPrev = this->pTail;
				this->pTail = node;
			}

			this->size += 1;

			return node;
		}

		void Delete(Node* node)
		{
			if (this->pHead == node && this->pTail == node)
			{
				this->pHead = this->pTail = NULL;
			}
			else if (this->pHead == node)
			{
				node->pNext->pPrev = NULL;
				this->pHead = node->pNext;
			}
			else if (this->pTail == node)
			{
				node->pPrev->pNext = NULL;
				this->pTail = node->pPrev;
			}
			else
			{
				node->pPrev->pNext = node->pNext;
				node->pNext->pPrev = node->pPrev;
			}

			delete node;
			this->size -= 1;

			return;
		}

		int Size()
		{
			return this->size;
		}
	};

	//종료시에 Thread 마다 생성한 vector를 제거하기 위해서, Thread 마다 생성한 vector의 전체 리스트를 가지고 있어야 한다.
	class RetireListRec
	{
	public:
		RetireListRec* pNext;
		SimpleList* rList;

	public:
		RetireListRec()
		{
			this->pNext = NULL;
			this->rList = NULL;
		}

		~RetireListRec()
		{
			delete this->rList;
		}

		static void Alloc(RetireListRec** pHead, SimpleList* rList)
		{
			RetireListRec* p = new RetireListRec();
			p->rList = rList;

			RetireListRec* oldHead;
			do
			{
				oldHead = *pHead;
				p->pNext = oldHead;
			} while (InterlockedCompareExchangePointer((PVOID*)pHead, p, oldHead) != oldHead);
		}

		static void DeleteAll(RetireListRec** pHead)
		{
			RetireListRec* p = *pHead;
			RetireListRec* n;
			while (p != NULL)
			{
				n = p->pNext;
				delete p;
				p = n;
			}
		}
	};

	class HP_helper
	{
	private:
		HpRec* pHpRecHead;
		RetireListRec* pRetireListHead;
		DWORD tlsSlot;

	private:
		SimpleList* GetRetireList()
		{
			SimpleList* rList = (SimpleList*)TlsGetValue(this->tlsSlot);
			if (rList == NULL)
			{
				rList = new SimpleList();
				TlsSetValue(this->tlsSlot, rList);
				RetireListRec::Alloc(&(this->pRetireListHead), rList);
			}

			return rList;
		}

		void Scan()
		{
			SimpleList* rList = GetRetireList();

			//현재 사용중인 Hazard Pointer들을 모은다.
			std::vector<PVOID> hpList;
			for (HpRec* p = this->pHpRecHead; p != NULL; p = p->pNext)
			{
				if (p->pNode != NULL)
					hpList.push_back(p->pNode);
			}

			//검색을 쉽게 하기 위해 일단 정렬을 수행한다.
			sort(hpList.begin(), hpList.end(), std::less<PVOID>());

			//현재 Thread의 RetireList에 있는 요소마다 검색을 수행한다.
			SimpleList::Node* p = rList->pHead;
			SimpleList::Node* temp;
			while (p != NULL)
			{
				//Hazard Pointer에 등록되지 않았다면
				if (binary_search(hpList.begin(), hpList.end(), p->pData) == false)
				{
					//메모리에서 삭제한다.
					delete p->pData;

					temp = p->pNext;
					rList->Delete(p);
					p = temp;
				}
				else
				{
					p = p->pNext;
				}
			}
		}

	public:
		HP_helper()
		{
			this->pHpRecHead = NULL;
			this->pRetireListHead = NULL;
			this->tlsSlot = TlsAlloc();
		}

		~HP_helper()
		{
			HpRec::DeleteAll(&(this->pHpRecHead));
			RetireListRec::DeleteAll(&(this->pRetireListHead));
			TlsFree(this->tlsSlot);
		}

		HpRec* Alloc_hp()
		{
			return HpRec::Alloc(&(this->pHpRecHead));
		}

		void Release_hp(HpRec* p)
		{
			HpRec::Release(p);
		}

		void RetireNode(PVOID pNode)
		{
			SimpleList* rList = GetRetireList();
			rList->Append(pNode);

			//너무 자주 Hp 검색 작업을 하지 않도록 Retire된 Node가 적절한 수가 되었을 경우에만 수행한다.
			if (rList->Size() >= 10)
				Scan();
		}

	};

	template<typename Tp>
	class shared_ptr;

	template<typename Tp>
	class weak_ptr;

	template<typename Tp>
	class enable_shared_from_this;

	template<typename Tp, typename = void>
	struct _Can_enable_shared : std::false_type
	{};

	template<typename Tp>
	struct _Can_enable_shared<Tp, std::void_t<typename Tp::_Esft_type>>
		: std::is_convertible<std::remove_cv_t<Tp> *, typename Tp::_Esft_type *>::type
	{};

	template<typename Tp>
	class control_block {

	private:

		mutable std::atomic_int use_count;
		mutable std::atomic_int	weak_count;

		Tp* ptr;

		HP_helper* hp;

		control_block(const control_block&) = delete;
		control_block& operator=(const control_block&) = delete;

		bool CAS(std::atomic_int* memory, int old_data, int new_data) const
		{
			int old_value = old_data;
			int new_value = new_data;

			return std::atomic_compare_exchange_strong
			(reinterpret_cast<std::atomic_int*>(memory), &old_value, new_value);
		}

	public:
		control_block() = delete;

		control_block(Tp* other, HP_helper* hp_helper)
			: ptr(other), use_count(1), weak_count(1), hp(hp_helper)
		{}

		virtual ~control_block()
		{}

		HpRec* Alloc_hp() const
		{
			return hp->Alloc_hp();
		}

		void Release_hp(HpRec* p) const
		{
			hp->Release_hp(p);
		}

		void enable_min_weak_count()
		{
			weak_count--;
		}

		control_block<Tp>* add_use_count()
		{
			int pred_value;

			while (true)
			{
				pred_value = use_count;

				if (pred_value > 0)
				{
					if (true == CAS(&use_count, pred_value, pred_value + 1))
						return this;

					else
						continue;
				}

				else
				{
					return nullptr;
				}
			}
		}

		void destroy(std::true_type)
		{
			delete ptr;
		}

		void destroy(std::false_type)
		{
			delete ptr;

			int curr_weak_count = weak_count;

			if (curr_weak_count == 1)
			{
				if (true == CAS(&weak_count, curr_weak_count, curr_weak_count - 1)) 
					hp->RetireNode(this);
			}
		}

		void release() noexcept
		{
			int curr_use_count;

			while (true)
			{
				curr_use_count = use_count;

				if (true == CAS(&use_count, curr_use_count, curr_use_count - 1))
				{
					if (curr_use_count == 1)
						destroy(std::bool_constant<std::conjunction_v<_Can_enable_shared<Tp>>>{});
					return;
				}

				else
					continue;
			}
		}

		Tp* get()
		{
			int curr_use_count = use_count;

			if (curr_use_count > 0)
				return ptr;

			else
				return nullptr;
		}

		int get_use_count()
		{
			int use_cnt = use_count;

			return use_cnt;
		}

#ifdef TEST_WEAK_PTR

		int get_weak_count()
		{
			int weak_cnt = weak_count;

			return weak_cnt;
		}

#endif // TEST_WEAK_PTR

		control_block<Tp>* add_weak_count() noexcept
		{
			int pred_value;

			while (true)
			{
				pred_value = weak_count;

				if (pred_value > 0)
				{
					if (true == CAS(&weak_count, pred_value, pred_value + 1))
						return this;

					else
						continue;
				}

				else
				{
					return nullptr;
				}
			}
		}

		void weak_release() noexcept
		{
			int curr_weak_count;

			while (true)
			{
				curr_weak_count = weak_count;

				if (true == CAS(&weak_count, curr_weak_count, curr_weak_count - 1))
				{
					if (curr_weak_count == 1)
						hp->RetireNode(this);

					return;
				}
			}
		}
	};

	template <typename Tp>
	class shared_ptr {

	public:
		using element_type = Tp;

	private:

		control_block<Tp>* ctr;

		friend class weak_ptr<Tp>;

		template<typename... Args>
		friend shared_ptr<Tp> make_shared(HP_helper*, Args&&... _Args);

		template<typename _Tp>
		friend bool operator==(const shared_ptr<_Tp>&, const shared_ptr<_Tp>&);

		void make_enable_shared_min_weak_count()
		{
			ctr->enable_min_weak_count();
		}

		void _Enable_shared_from_this(const shared_ptr<Tp>& _This, Tp* _Ptr, std::true_type)
		{
			enable_shared_from_this<Tp>* enable_shared_from_this_base
				= reinterpret_cast<enable_shared_from_this<Tp>*>(_Ptr);

			enable_shared_from_this_base->Wptr = _This;
			make_enable_shared_min_weak_count();
		}

		void _Enable_shared_from_this(const shared_ptr<Tp>&, Tp*, std::false_type)
		{}

		HpRec* Alloc_hp() const
		{
			return ctr->Alloc_hp();
		}

		void Release_hp(HpRec* p) const
		{
			ctr->Release_hp(p);
		}

	public:
		void _Set_ctr_and_enable_shared(Tp* new_ptr, control_block<Tp>* new_ctr)
		{
			ctr = new_ctr;
			_Enable_shared_from_this(*this, new_ptr, std::bool_constant<std::conjunction_v<_Can_enable_shared<Tp>>>{});
		}

	private:
		bool CAS(control_block<Tp>** memory, control_block<Tp>* oldaddr, control_block<Tp>* newaddr)
		{
			long old_addr = reinterpret_cast<long>(oldaddr);
			long new_addr = reinterpret_cast<long>(newaddr);

			return std::atomic_compare_exchange_strong
			(reinterpret_cast<std::atomic_long*>(memory), &old_addr, new_addr);
		}

		control_block<Tp>* add_shared_copy() const
		{
			control_block<Tp>* curr_ctr;
			control_block<Tp>* ret_ctr = nullptr;
			
			HpRec* hpRec = Alloc_hp();

			do {
				curr_ctr = ctr;
				if (curr_ctr == nullptr) {
					Release_hp(hpRec);
					return nullptr;
				}

				hpRec->pNode = curr_ctr;

				if (curr_ctr != ctr)
					continue;

				ret_ctr = curr_ctr->add_use_count();

			} while (ret_ctr == nullptr);

			Release_hp(hpRec);
			return ret_ctr;
		}

		control_block<Tp>* add_weak_copy() const
		{
			control_block<Tp>* curr_ctr;
			control_block<Tp>* ret_ctr = nullptr;

			HpRec* hpRec = Alloc_hp();

			do {
				curr_ctr = ctr;
				if (curr_ctr == nullptr) {
					Release_hp(hpRec);
					return nullptr;
				}

				hpRec->pNode = curr_ctr;

				if (curr_ctr != ctr)
					continue;

				ret_ctr = curr_ctr->add_weak_count();
			} while (ret_ctr == nullptr);

			Release_hp(hpRec);
			return ret_ctr;
		}

		Tp* get() const
		{
			control_block<Tp>* curr_ctr = ctr;

			if (curr_ctr != nullptr)
				return curr_ctr->get();
			else
				return nullptr;
		}

	public:

		shared_ptr()
			: ctr(nullptr)
		{}

		shared_ptr(nullptr_t)
			: shared_ptr()
		{}

		shared_ptr(Tp* other)
			: ctr(new control_block<Tp>(other))
		{}

		shared_ptr(const shared_ptr& other)
		{
			ctr = other.add_shared_copy();
		}

		shared_ptr(const weak_ptr<Tp>& other)
		{
			ctr = other.add_shared_copy();
		}

		shared_ptr& operator=(nullptr_t)
		{
			control_block<Tp>* pred_ctr;

			while (true)
			{
				pred_ctr = ctr;

				if (pred_ctr == nullptr)
					return *this;

				if (true == CAS(&ctr, pred_ctr, nullptr))
				{
					pred_ctr->release();
					return *this;
				}
			}
		}

		shared_ptr& operator=(const shared_ptr& other)
		{
			control_block<Tp>* pred_ctr;
			control_block<Tp>* other_ctr;

			while (true)
			{
				pred_ctr = ctr;
				other_ctr = other.add_shared_copy();

				if (other_ctr == nullptr)
				{
					if (true == CAS(&ctr, pred_ctr, nullptr))
					{
						if (pred_ctr != nullptr)
							pred_ctr->release();
						return *this;
					}

					else
						continue;
				}

				if (other_ctr == pred_ctr)
				{
					other_ctr->release();
					return *this;
				}

				if (true == CAS(&ctr, pred_ctr, other_ctr))
				{
					if (pred_ctr != nullptr)
						pred_ctr->release();

					return *this;
				}
				else
				{
					other_ctr->release();
					continue;
				}
			}
		}

		~shared_ptr()
		{
			control_block<Tp>* curr_ctr = ctr;

			if (curr_ctr != nullptr)
				curr_ctr->release();

		}

		Tp& operator*()
		{
			return *get();
		}

		Tp* operator->() const
		{
			return get();
		}

		operator bool()
		{
			control_block<Tp>* curr_ctr = get();

			if (curr_ctr != nullptr)
				if (curr_ctr->get_use_count() > 0)
					return true;

			return false;
		}

		int use_count()
		{
			control_block<Tp>* curr_ctr = get();

			if (curr_ctr != nullptr)
				return ctr->get_use_count();

			return 0;
		}

#ifdef TEST_WEAK_PTR
		int weak_count()
		{
			return ctr->get_weak_count();
		}
#endif // TEST_WEAK_PTR
		void reset()
		{
			control_block<Tp>* pred_ctr;

			while (true) {
				pred_ctr = get();

				if (pred_ctr == nullptr)
					return;

				if (true == CAS(&ctr, pred_ctr, nullptr)) {
					pred_ctr->release();
					return;
				}

				else
					continue;
			}
		}
	};

	template<typename _Tp>
	bool operator==(const shared_ptr<_Tp>& __a, const shared_ptr<_Tp>& __b)
	{
		return __a.get() == __b.get();
	}

	template<typename _Tp>
	bool operator==(const shared_ptr<_Tp>& __a, nullptr_t)
	{
		return (__a.get() == nullptr);
	}

	template<typename _Tp>
	bool operator==(nullptr_t, const shared_ptr<_Tp>& __a)
	{
		return (__a.get() == nullptr);
	}

	template<typename _Tp>
	bool operator!=(const shared_ptr<_Tp>& __a, const shared_ptr<_Tp>& __b)
	{
		return !(operator==(__a, __b));
	}

	template<typename _Tp>
	bool operator!=(const shared_ptr<_Tp>& __a, nullptr_t)
	{
		return (__a.get() != nullptr);
	}

	template<typename _Tp>
	bool operator!=(nullptr_t, const shared_ptr<_Tp>& __a)
	{
		return (__a.get() != nullptr);
	}

	template<class _Elem, class _Traits, class _Ty>
	std::basic_ostream<_Elem, _Traits>& operator<<(std::basic_ostream<_Elem, _Traits>& _Out, const shared_ptr<_Ty>& _Px)
	{
		return (_Out << _Px.get());
	}

	template<typename _Tp, typename... Args>
	shared_ptr<_Tp> make_shared(HP_helper* helper, Args&&... _Args)
	{
		_Tp* new_Tp = new _Tp(std::forward<Args>(_Args)...);
		control_block<_Tp>* new_ctr = new control_block<_Tp>(new_Tp, helper);

		shared_ptr<_Tp> _Ret;
		_Ret._Set_ctr_and_enable_shared(new_Tp, new_ctr);

		return _Ret;
	}

	template<typename Tp>
	class weak_ptr {
	private:

		control_block<Tp>* ctr;

		friend class shared_ptr<Tp>;

		friend class enable_shared_from_this<Tp>;

		HpRec* Alloc_hp() const
		{
			return ctr->Alloc_hp();
		}

		void Release_hp(HpRec* p) const
		{
			ctr->Release_hp(p);
		}

		control_block<Tp>* add_shared_copy() const
		{
			control_block<Tp>* curr_ctr;
			control_block<Tp>* ret_ctr = nullptr;

			HpRec* hpRec = Alloc_hp();

			do {
				curr_ctr = ctr;
				if (curr_ctr == nullptr) {
					Release_hp(hpRec);
					return nullptr;
				}

				hpRec->pNode = curr_ctr;

				if (curr_ctr != ctr)
					continue;

				ret_ctr = curr_ctr->add_use_count();
			} while (ret_ctr == nullptr);
			
			Release_hp(hpRec);
			return ret_ctr;
		}

		control_block<Tp>* add_weak_copy() const
		{
			control_block<Tp>* curr_ctr;
			control_block<Tp>* ret_ctr = nullptr;

			HpRec* hpRec = Alloc_hp();

			do {
				curr_ctr = ctr;
				if (curr_ctr == nullptr) {
					Release_hp(hpRec);
					return nullptr;
				}

				hpRec->pNode = curr_ctr;

				if (curr_ctr != ctr)
					continue;

				ret_ctr = curr_ctr->add_weak_count();
			} while (ret_ctr == nullptr);

			Release_hp(hpRec);
			return ret_ctr;
		}

		bool CAS(control_block<Tp>** memory, control_block<Tp>* oldaddr, control_block<Tp>* newaddr)
		{
			long old_addr = reinterpret_cast<long>(oldaddr);
			long new_addr = reinterpret_cast<long>(newaddr);

			return std::atomic_compare_exchange_strong
			(reinterpret_cast<std::atomic_long*>(memory), &old_addr, new_addr);
		}

		Tp* get() const
		{
			control_block<Tp>* curr_ctr = ctr;

			if (curr_ctr != nullptr)
				return curr_ctr->get();
			else
				return nullptr;
		}

	public:

		weak_ptr()
			: ctr(nullptr)
		{}

		weak_ptr(nullptr_t)
			: weak_ptr()
		{}

		weak_ptr(const shared_ptr<Tp>& other)
		{
			ctr = other.add_weak_copy();
		}

		weak_ptr(const weak_ptr<Tp>& other)
		{
			ctr = other.add_weak_copy();
		}

		~weak_ptr()
		{
			control_block<Tp>* curr_ctr = ctr;

			if (curr_ctr != nullptr)
				curr_ctr->weak_release();
		}

		weak_ptr& operator=(nullptr_t)
		{
			control_block<Tp>* pred_ctr;

			while (true)
			{
				pred_ctr = ctr;
				if (true == CAS(&ctr, pred_ctr, nullptr))
				{
					if (pred_ctr != nullptr)
						pred_ctr->weak_release();
					return *this;
				}
			}
		}

		weak_ptr& operator=(const shared_ptr<Tp>& other)
		{
			control_block<Tp>* pred_ctr;
			control_block<Tp>* other_ctr;

			while (true)
			{
				pred_ctr = ctr;
				other_ctr = other.add_weak_copy();

				if (other_ctr == nullptr)
				{
					if (true == CAS(&ctr, pred_ctr, nullptr))
					{
						if (pred_ctr != nullptr)
							pred_ctr->weak_release();
						return *this;
					}

					else
						continue;
				}

				if (other_ctr == pred_ctr)
				{
					other_ctr->weak_release();
					return *this;
				}

				if (true == CAS(&ctr, pred_ctr, other_ctr))
				{
					if (pred_ctr != nullptr)
						pred_ctr->weak_release();

					return *this;
				}
				else
				{
					other_ctr->weak_release();
					continue;
				}
			}
		}

		weak_ptr& operator=(const weak_ptr& other)
		{
			control_block<Tp>* pred_ctr;
			control_block<Tp>* other_ctr;

			while (true)
			{
				pred_ctr = ctr;
				other_ctr = other.add_weak_copy();

				if (other_ctr == nullptr)
				{
					if (true == CAS(&ctr, pred_ctr, nullptr))
					{
						if (pred_ctr != nullptr)
							pred_ctr->weak_release();
						return *this;
					}

					else
						continue;
				}

				if (other_ctr == pred_ctr)
				{
					other_ctr->weak_release();
					return *this;
				}

				if (true == CAS(&ctr, pred_ctr, other_ctr))
				{
					if (pred_ctr != nullptr)
						pred_ctr->weak_release();

					return *this;
				}
				else
				{
					other_ctr->weak_release();
					continue;
				}
			}
		}

		shared_ptr<Tp> lock() const
		{
			Tp* ret = get();

			if (ret != nullptr)
				return shared_ptr<Tp>(*this);

			else
				return shared_ptr<Tp>();
		}

		int use_count()
		{
			control_block<Tp>* curr = get();

			if (curr != nullptr)
				return curr->get_use_count();

			return 0;
		}

#ifdef TEST_WEAK_PTR
		int weak_count()
		{
			return ctr->get_weak_count();
		}

		Tp* get_ptr() const
		{
			return ctr->get();
		}
#endif // TEST_WEAK_PTR

		bool expired()
		{
			control_block<Tp>* pred_ctr = get();
			if (pred_ctr != nullptr)
				return (pred_ctr->get_use_count() == 0);

			return false;
		}

		void reset()
		{
			control_block<Tp>* pred_ctr;

			while (true) {
				pred_ctr = get();

				if (pred_ctr == nullptr)
					return;

				if (true == CAS(&ctr, pred_ctr, nullptr)) {
					pred_ctr->weak_release();
					return;
				}

				else
					continue;
			}
		}
	};


	template<typename Tp>
	class enable_shared_from_this {

	private:
		weak_ptr<Tp> Wptr;

		friend shared_ptr<Tp>;

	public:
		using _Esft_type = enable_shared_from_this;

		shared_ptr<Tp> shared_from_this()
		{
			return (shared_ptr<Tp>(Wptr));
		}

		weak_ptr<Tp> weak_from_this()
		{
			return Wptr;
		}

	protected:
		enable_shared_from_this()
			: Wptr()
		{}

		enable_shared_from_this(const enable_shared_from_this& other)
			: Wptr()
		{}

		enable_shared_from_this& operator=(const enable_shared_from_this&)
		{
			return (*this);
		}

		~enable_shared_from_this() = default;
	};
}