#pragma once

#include <atomic>
#include <mutex>
#include <Windows.h>

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
	class ctr_block {

	private:

		mutable std::atomic_int use_count;
		mutable std::atomic_int	weak_count;

		Tp* ptr;

		ctr_block(const ctr_block&) = delete;
		ctr_block& operator=(const ctr_block&) = delete;

		bool CAS(std::atomic_int* memory, int old_data, int new_data) const
		{
			int old_value = old_data;
			int new_value = new_data;
			
			return std::atomic_compare_exchange_strong
			(reinterpret_cast<std::atomic_int*>(memory), &old_value, new_value);
		}

	public:
		ctr_block() = delete;

		ctr_block(Tp* other)
			: ptr(other), use_count(1), weak_count(0)
		{}

		virtual ~ctr_block()
		{}

		ctr_block<Tp>* add_ref_copy() 
		{
			int pred_value;

			while (true)
			{
				pred_value = use_count;
				if (pred_value > 0)
				{
					if (true == CAS(&use_count, pred_value, pred_value + 1))
						return this;
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
			ptr = nullptr;
		}

		void destroy(std::false_type)
		{
			delete ptr;
			ptr = nullptr;

			int curr_weak_count = weak_count;
			
			if (curr_weak_count == 0)
				delete this;
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
			}
		}

		Tp* get()
		{
			int curr_use_count = use_count;
			
			if (use_count > 0)
				return ptr;
			else
				return nullptr;
		}

		int get_use_count()	
		{
			int use_cnt;
			do
			{
				use_cnt = use_count;
			} while (!std::atomic_compare_exchange_strong(&use_count, &use_cnt, use_cnt));

			return use_cnt;
		}

		void weak_add_ref() noexcept
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
						delete this;

					return;
				}
			}
		}
	};

	template <typename Tp>
	class shared_ptr {

	private:

		ctr_block<Tp>* ctr;
		mutable std::mutex sp_lock;

		template<typename Tp>
		friend class weak_ptr;

		template<typename Tp, typename... Args>
		friend shared_ptr<Tp> make_shared(Args&&... _Args);

		template<typename Tp>
		void _Set_ptr_rep(Tp* other_ptr, ctr_block<Tp>* other_ctr)
		{
			//ptr = other_ptr;
			ctr = other_ctr;
		}

		template<typename Tp>
		void _Set_ptr_rep_and_enable_shared(Tp* other_ptr, ctr_block<Tp>* other_ctr)
		{
			this->_Set_ptr_rep(other_ptr, other_ctr);
			_Enable_shared_from_this(*this, other_ptr);
		}

		bool CAS(ctr_block<Tp>** memory, ctr_block<Tp>* oldaddr, ctr_block<Tp>* newaddr)
		{
			int old_addr = reinterpret_cast<int>(oldaddr);
			int new_addr = reinterpret_cast<int>(newaddr);

			return std::atomic_compare_exchange_strong
			(reinterpret_cast<std::atomic_int*>(memory), &old_addr, new_addr);
		}

	public:
		Tp* get() const
		{
			ctr_block<Tp>* curr_ctr = ctr;

			if (curr_ctr != nullptr)
				return curr_ctr->get();
			else
				return nullptr;
		}

	public:

		void lock() const
		{
			sp_lock.lock();
		}

		void unlock() const
		{
			sp_lock.unlock();
		}

		shared_ptr()
			: ctr(nullptr)
		{}

		shared_ptr(nullptr_t)
			: shared_ptr()
		{}

		shared_ptr(Tp* other)
			: ctr(new ctr_block<Tp>(other))
		{}

		ctr_block<Tp>* add_ref_copy() const
		{
			ctr_block<Tp>* curr_ctr;
			ctr_block<Tp>* ret_ctr;
			
			do {
				curr_ctr = ctr;
				if (curr_ctr == nullptr)
					return nullptr;

				ret_ctr = curr_ctr->add_ref_copy();
			} while (ret_ctr == nullptr);

			return ret_ctr;
		}

		shared_ptr(const shared_ptr& other)
		{
			ctr = other.add_ref_copy();
		}

		shared_ptr(shared_ptr& other)
		{
			ctr = other.add_ref_copy();
		}

		shared_ptr& operator=(nullptr_t)
		{
			ctr_block<Tp>* pred_ctr;

			while (true)
			{
				pred_ctr = ctr;
				if (true == CAS(&ctr, pred_ctr, nullptr))
				{
					if(pred_ctr != nullptr)
						pred_ctr->release();
					return *this;
				}
			}
		}

		shared_ptr& operator=(const shared_ptr& other)
		{
			ctr_block<Tp>* pred_ctr;
			ctr_block<Tp>* other_ctr;

			while (true)
			{
				pred_ctr = ctr;
				other_ctr = other.add_ref_copy();

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

		shared_ptr& operator=(shared_ptr& other)
		{
			ctr_block<Tp>* pred_ctr;
			ctr_block<Tp>* other_ctr;

			while (true)
			{
				pred_ctr = ctr;
				other_ctr = other.add_ref_copy();

				if (other_ctr == nullptr)
				{
					if (true == CAS(&ctr, pred_ctr, nullptr))
					{
						if(pred_ctr != nullptr)
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
			ctr_block<Tp>* curr_ctr = ctr;

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
			ctr_block<Tp>* curr_ctr = get();

			if (curr_ctr != nullptr)
				return true;

			else
				return false;
		}

		int use_count()
		{
			return ctr->get_use_count();
		}

		void reset()		// ***************
		{
			if (!ctr)	return;

			ctr->lock();

			ctr_block<Tp>* curr_ctr = ctr;

			//ptr = nullptr;
			ctr = nullptr;

			if (!curr_ctr->release())
				curr_ctr->unlock();
		}

		void swap(shared_ptr& other) // *** lock
		{
			
			/*Tp* emp_p;
			ctr_block<Tp>* emp_c;

			if(ctr)
				ctr->lock();
			if(other.ctr)
				other.ctr->lock();

			emp_p = ptr;
			ptr = other.ptr;
			other.ptr = emp_p;

			emp_c = ctr;
			ctr = other.ctr;
			other.ctr = emp_c;

			if(other.ctr)
				other.ctr->unlock();
			if(ctr)
				ctr->unlock();*/
		}
	};

	template<typename Tp1, typename Tp2>
	bool operator==(const shared_ptr<Tp1>& __a, const shared_ptr<Tp2>& __b)
	{
		return __a.get() == __b.get();
	}

	template<typename Tp>
	bool operator==(const shared_ptr<Tp>& __a, nullptr_t)
	{
		return !(&__a);
	}

	template<typename Tp>
	bool operator==(nullptr_t, const shared_ptr<Tp>& __a)
	{
		return !(&__a);
	}

	template<typename Tp1, typename Tp2>
	bool operator!=(const shared_ptr<Tp1>& __a, const shared_ptr<Tp2>& __b)
	{
		return __a.get() != __b.get();
	}

	template<typename Tp>
	bool operator!=(const shared_ptr<Tp>& __a, nullptr_t)
	{
		return &__a;
	}

	template<typename Tp>
	bool operator!=(nullptr_t, const shared_ptr<Tp>& __a)
	{
		return &__a;
	}

	template<class _Elem, class _Traits, class _Ty>
	std::basic_ostream<_Elem, _Traits>& operator<<(std::basic_ostream<_Elem, _Traits>& _Out, const shared_ptr<_Ty>& _Px)
	{	
		return (_Out << _Px.get());
	}

	template<typename Tp, typename... Args>
	shared_ptr<Tp> make_shared(Args&&... _Args)
	{
		Tp* new_Tp = new Tp(std::forward<Args>(_Args)...);
		ctr_block<Tp>* new_ctr = new ctr_block<Tp>(new_Tp);

		shared_ptr<Tp> _Ret;
		_Ret._Set_ptr_rep_and_enable_shared(new_Tp, new_ctr);

		return (_Ret);
	}

	template<typename Tp>
	void _Enable_shared_from_this1(const shared_ptr<Tp>& _This, Tp* _Ptr, std::true_type)
	{	
		enable_shared_from_this<Tp>* enable_shared_from_this_base
			= reinterpret_cast<enable_shared_from_this<Tp>*>(_Ptr);
		enable_shared_from_this_base->Wptr = _This;

	}

	template<typename Tp>
	void _Enable_shared_from_this1(const shared_ptr<Tp>&, Tp*, std::false_type)
	{}
	template<typename Tp>
	void _Enable_shared_from_this(const shared_ptr<Tp>& _This, Tp * _Ptr)
	{
		_Enable_shared_from_this1(_This, _Ptr,
			std::bool_constant<std::conjunction_v<_Can_enable_shared<Tp>>>{});
	}

	template<typename Tp>
	class weak_ptr{

	private:

		Tp* ptr;
		ctr_block<Tp>* ctr;
		mutable std::mutex sp_lock;

		template<typename Tp>
		friend class shared_ptr;

		template<typename Tp>
		friend class enable_shared_from_this;

	public:

		void lock() const
		{
			sp_lock.lock();
		}

		void unlock() const
		{
			sp_lock.unlock();
		}

		weak_ptr()
			: ptr(nullptr), ctr(nullptr)
		{}

		weak_ptr(nullptr_t)
			: weak_ptr()
		{}

		weak_ptr(const shared_ptr<Tp>& other)
		{

			other.lock();
			other.ctr->weak_add_ref();

			ptr = other.ptr;
			ctr = other.ctr;
			other.unlock();
		}

		weak_ptr(const weak_ptr<Tp>& other)
		{

			other.lock();
			other.ctr->weak_add_ref();

			ptr = other.ptr;
			ctr = other.ctr;
			other.unlock();
		}

		~weak_ptr()
		{

			lock();
			if (ctr)	ctr->weak_release();
			unlock();
		}

		weak_ptr& operator=(nullptr_t)
		{
			lock();

			if (ctr) {
				ctr->weak_release();
				ctr = nullptr;
				ptr = nullptr;
			}

			unlock();

			return *this;
		}

		weak_ptr& operator=(const shared_ptr<Tp>& other)
		{
			lock();
			if (other.ptr == ptr) {
				unlock();
				return *this;
			}

			other.lock();

			other.ctr->weak_add_ref();

			ctr_block<Tp>* curr_ctr = ctr;
			ptr = other.ptr;
			ctr = other.ctr;

			other.unlock();

			if (curr_ctr) curr_ctr->weak_release();
			unlock();

			return *this;
		}

		weak_ptr& operator=(const weak_ptr& other)
		{
			lock();
			if (other.ptr == ptr) {
				unlock();
				return *this;
			}

			other.lock();

			other.ctr->weak_add_ref();

			ctr_block<Tp>* curr_ctr = ctr;
			ptr = other.ptr;
			ctr = other.ctr;

			other.unlock();

			if (curr_ctr) curr_ctr->weak_release();
			unlock();

			return *this;
		}

		shared_ptr<Tp> bl_lock() const
		{
			if (ptr)
				return shared_ptr<Tp>(*this);

			else
				return nullptr;
		}

		void reset()	// ***********************
		{
			if (!ctr)	return;

			ctr->lock();

			ctr_block<Tp>* curr_ctr = ctr;

			ptr = nullptr;
			ctr = nullptr;

			if (!curr_ctr->weak_release())
				curr_ctr->unlock();
		}

		void swap(weak_ptr& other) {
			Tp* emp_p;
			ctr_block<Tp>* emp_c;

			ctr->lock();
			other.ctr->lock();

			emp_p = ptr;
			ptr = other.ptr;
			other.ptr = emp_p;

			emp_c = ctr;
			ctr = other.ctr;
			other.ctr = emp_c;

			other.ctr->unlock();
			ctr->unlock();
		}
	};


	template<typename Tp>
	class enable_shared_from_this{

	private:
		weak_ptr<Tp> Wptr;

		template<typename Tp>
		friend void _Enable_shared_from_this1(const shared_ptr<Tp>& _This, Tp* _Ptr, std::true_type);

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

	template<class Tp>
	inline shared_ptr<Tp> atomic_load_explicit(const shared_ptr<Tp> * _Ptr,
		std::memory_order)
	{	// load *_Ptr atomically
		std::_Shared_ptr_spin_lock _Lock;
		shared_ptr<Tp> _Result = *_Ptr;
		return (_Result);
	}

	template<class Tp>
	inline shared_ptr<Tp> atomic_load(const shared_ptr<Tp> * _Ptr)
	{	// load *_Ptr atomically
		return (atomic_load_explicit(_Ptr, std::memory_order_seq_cst));
	}

	template<class Tp> inline
		void atomic_store_explicit(shared_ptr<Tp> * _Ptr, shared_ptr<Tp> _Other,
			std::memory_order)
	{	// store _Other to *_Ptr atomically
		std::_Shared_ptr_spin_lock _Lock;
		_Ptr->swap(_Other);
	}

	template<class Tp> inline
		void atomic_store(shared_ptr<Tp> * _Ptr, shared_ptr<Tp> _Other)
	{	// store _Other to *_Ptr atomically
		atomic_store_explicit(_Ptr, _Other, std::memory_order_seq_cst);
	}


}