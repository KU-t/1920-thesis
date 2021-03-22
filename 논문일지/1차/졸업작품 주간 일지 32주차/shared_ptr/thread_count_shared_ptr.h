#pragma once

//#define TEST_WEAK_PTR

#include <atomic>
#include <mutex>

namespace LFSP {

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

		control_block(Tp* other)
			: ptr(other), use_count(1), weak_count(1)
		{}

		virtual ~control_block()
		{}

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

		void destroy(std::true_type, const shared_ptr<Tp>& p)
		{
			delete ptr;
		}

		void destroy(std::false_type, const shared_ptr<Tp>& sp)
		{
			delete ptr;

			int curr_weak_count = weak_count;

			if (curr_weak_count == 1)
			{
				if (true == CAS(&weak_count, curr_weak_count, curr_weak_count - 1))
				{
					//sp.deleted = true;

					while (true)
					{
						int curr_thread_count = sp.thread_count;
						if (false == CAS(&sp.thread_count, curr_thread_count, curr_thread_count - 1))
							continue;

						if (curr_thread_count == 1)
							delete this;
						
						//sp.thread_count = 1;
						//sp.deleted = false;
						return;
					}
				}
			}
		}

		void release(const shared_ptr<Tp>& sp) noexcept
		{
			int curr_use_count;

			while (true)
			{
				curr_use_count = use_count;

				if (true == CAS(&use_count, curr_use_count, curr_use_count - 1))
				{
					if (curr_use_count == 1)
						destroy(std::bool_constant<std::conjunction_v<_Can_enable_shared<Tp>>>{}, sp);

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

		void weak_release(weak_ptr<Tp>& sp) noexcept
		{
			int curr_weak_count;

			while (true)
			{
				curr_weak_count = weak_count;

				if (true == CAS(&weak_count, curr_weak_count, curr_weak_count - 1))
				{
					sp.deleted = true;

					while (true)
					{
						int curr_thread_count = sp.thread_count;
						if (false == CAS(&sp.thread_count, curr_thread_count, curr_thread_count - 1))
							continue;

						if (curr_thread_count == 1)
							delete this;
						
						sp.thread_count = 1;
						sp.deleted = false;
						return;
					}
				}
			}
		}
	};

	template <typename Tp>
	class shared_ptr {

	public:
		using element_type = Tp;

	private:

		volatile bool deleted;
		control_block<Tp>* ctr;

		mutable std::atomic_int thread_count;

		friend class control_block<Tp>;
		friend class weak_ptr<Tp>;

		template<typename... Args>
		friend shared_ptr<Tp> make_shared(Args&&... _Args);

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

	public:
		void _Set_ctr_and_enable_shared(Tp* new_ptr, control_block<Tp>* new_ctr)
		{
			ctr = new_ctr;
			thread_count = 1;
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

		bool TCAS(std::atomic_int* memory, int old_data, int new_data) const
		{
			int old_value = old_data;
			int new_value = new_data;

			return std::atomic_compare_exchange_strong
			(reinterpret_cast<std::atomic_int*>(memory), &old_value, new_value);
		}

		control_block<Tp>* add_shared_copy() const
		{
			control_block<Tp>* curr_ctr;
			control_block<Tp>* ret_ctr;
			int curr_thread_count;

			while (true)
			{

				curr_ctr = ctr;
				if (curr_ctr == nullptr)
					return nullptr;

				curr_thread_count = thread_count;
				if (curr_thread_count == 0)
					return nullptr;

				if (true == TCAS(&thread_count, curr_thread_count, curr_thread_count + 1))
					break;
			}

			ret_ctr = curr_ctr->add_use_count();

			while (true)
			{
				curr_thread_count = thread_count;
				if (false == TCAS(&thread_count, curr_thread_count, curr_thread_count - 1))
					continue;

				if (curr_thread_count == 1)
				{
					delete curr_ctr;
					return nullptr;
				}

				else
					return ret_ctr;
			}
		}

		control_block<Tp>* add_weak_copy() const
		{
			control_block<Tp>* curr_ctr;
			control_block<Tp>* ret_ctr;
			int curr_thread_count;

			while (true)
			{

				curr_ctr = ctr;
				if (curr_ctr == nullptr)
					return nullptr;

				curr_thread_count = thread_count;
				if (curr_thread_count == 0)
					return nullptr;

				if (true == TCAS(&thread_count, curr_thread_count, curr_thread_count + 1))
					break;
			}

			ret_ctr = curr_ctr->add_weak_count();

			while (true)
			{
				curr_thread_count = thread_count;
				if (false == TCAS(&thread_count, curr_thread_count, curr_thread_count - 1))
					continue;

				if (curr_thread_count == 1)
				{
					delete curr_ctr;
					return nullptr;
				}

				else
					return ret_ctr;
			}
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
			: ctr(nullptr), thread_count(1), deleted(false)
		{}

		shared_ptr(nullptr_t)
			: shared_ptr()
		{}

		shared_ptr(Tp* other)
			: ctr(new control_block<Tp>(other)), thread_count(1), deleted(false)
		{}

		shared_ptr(const shared_ptr& other)
		{
			thread_count = 1;
			deleted = false;
			ctr = other.add_shared_copy();
		}

		shared_ptr(const weak_ptr<Tp>& other)
		{
			thread_count = 1;
			deleted = false; 
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
					pred_ctr->release(*this);
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

				if (other.deleted == false)
				{
					if (true == CAS(&ctr, pred_ctr, nullptr))
					{
						if (pred_ctr != nullptr)
							pred_ctr->release(*this);
						return *this;
					}

					else
						continue;
				}

				other_ctr = other.add_shared_copy();

				if (other_ctr == nullptr)
				{
					if (true == CAS(&ctr, pred_ctr, nullptr))
					{
						if (pred_ctr != nullptr)
							pred_ctr->release(*this);
						return *this;
					}

					else
						continue;
				}

				if (other_ctr == pred_ctr)
				{
					other_ctr->release(*this);
					return *this;
				}

				if (true == CAS(&ctr, pred_ctr, other_ctr))
				{
					if (pred_ctr != nullptr)
						pred_ctr->release(*this);

					return *this;
				}
				else
				{
					other_ctr->release(*this);
					continue;
				}
			}
		}

		~shared_ptr()
		{
			control_block<Tp>* curr_ctr = ctr;

			if (curr_ctr != nullptr)
				curr_ctr->release(*this);

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
	shared_ptr<_Tp> make_shared(Args&&... _Args)
	{
		_Tp* new_Tp = new _Tp(std::forward<Args>(_Args)...);
		control_block<_Tp>* new_ctr = new control_block<_Tp>(new_Tp);

		shared_ptr<_Tp> _Ret;
		_Ret._Set_ctr_and_enable_shared(new_Tp, new_ctr);

		return _Ret;
	}

	template<typename Tp>
	class weak_ptr {
	private:

		volatile bool deleted;
		control_block<Tp>* ctr;
		std::atomic_int thread_count;

		friend class control_block<Tp>;
		friend class shared_ptr<Tp>;

		friend class enable_shared_from_this<Tp>;

		control_block<Tp>* add_shared_copy() const
		{
			control_block<Tp>* curr_ctr;
			control_block<Tp>* ret_ctr;
			int curr_thread_count;

			while (true)
			{

				curr_ctr = ctr;
				if (curr_ctr == nullptr)
					return nullptr;

				curr_thread_count = thread_count;
				if (curr_thread_count == 0)
					return nullptr;

				if (true == CAS(&thread_count, curr_thread_count, curr_thread_count + 1))
					break;
			}

			ret_ctr = curr_ctr->add_use_count();

			while (true)
			{
				curr_thread_count = thread_count;
				if (false == CAS(&thread_count, curr_thread_count, curr_thread_count - 1))
					continue;

				if (curr_thread_count == 1)
				{
					delete curr_ctr;
					return nullptr;
				}

				else
					return ret_ctr;
			}
		}

		control_block<Tp>* add_weak_copy() const
		{
			control_block<Tp>* curr_ctr;
			control_block<Tp>* ret_ctr;
			int curr_thread_count;

			while (true)
			{

				curr_ctr = ctr;
				if (curr_ctr == nullptr)
					return nullptr;

				curr_thread_count = thread_count;
				if (curr_thread_count == 0)
					return nullptr;

				if (true == CAS(&thread_count, curr_thread_count, curr_thread_count + 1))
					break;
			}

			ret_ctr = curr_ctr->add_weak_count();

			while (true)
			{
				curr_thread_count = thread_count;
				if (false == CAS(&thread_count, curr_thread_count, curr_thread_count - 1))
					continue;

				if (curr_thread_count == 1)
				{
					delete curr_ctr;
					return nullptr;
				}

				else
					return ret_ctr;
			}
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
			: ctr(nullptr), thread_count(1), deleted(false)
		{}

		weak_ptr(nullptr_t)
			: weak_ptr()
		{}

		weak_ptr(const shared_ptr<Tp>& other)
		{
			thread_count = 1;
			deleted = false;
			ctr = other.add_weak_copy();
		}

		weak_ptr(const weak_ptr<Tp>& other)
		{
			thread_count = 1;
			deleted = false;
			ctr = other.add_weak_copy();
		}

		~weak_ptr()
		{
			control_block<Tp>* curr_ctr = ctr;

			if (curr_ctr != nullptr)
				curr_ctr->weak_release(*this);
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
						pred_ctr->weak_release(*this);
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

				if (other.deleted == false)
				{
					if (true == CAS(&ctr, pred_ctr, nullptr))
					{
						if (pred_ctr != nullptr)
							pred_ctr->release(*this);
						return *this;
					}

					else
						continue;
				}

				other_ctr = other.add_weak_copy();

				if (other_ctr == nullptr)
				{
					if (true == CAS(&ctr, pred_ctr, nullptr))
					{
						if (pred_ctr != nullptr)
							pred_ctr->weak_release(*this);
						return *this;
					}

					else
						continue;
				}

				if (other_ctr == pred_ctr)
				{
					other_ctr->weak_release(*this);
					return *this;
				}

				if (true == CAS(&ctr, pred_ctr, other_ctr))
				{
					if (pred_ctr != nullptr)
						pred_ctr->weak_release(*this);

					return *this;
				}
				else
				{
					other_ctr->weak_release(*this);
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

				if (other.deleted == false)
				{
					if (true == CAS(&ctr, pred_ctr, nullptr))
					{
						if (pred_ctr != nullptr)
							pred_ctr->release(*this);
						return *this;
					}

					else
						continue;
				}

				other_ctr = other.add_weak_copy();

				if (other_ctr == nullptr)
				{
					if (true == CAS(&ctr, pred_ctr, nullptr))
					{
						if (pred_ctr != nullptr)
							pred_ctr->weak_release(*this);
						return *this;
					}

					else
						continue;
				}

				if (other_ctr == pred_ctr)
				{
					other_ctr->weak_release(*this);
					return *this;
				}

				if (true == CAS(&ctr, pred_ctr, other_ctr))
				{
					if (pred_ctr != nullptr)
						pred_ctr->weak_release(*this);

					return *this;
				}
				else
				{
					other_ctr->weak_release(*this);
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
					pred_ctr->weak_release(*this);
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