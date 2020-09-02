#pragma once

#include <atomic>
#include <mutex>

namespace LSP {

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

		std::atomic_int use_count;
		std::atomic_int	weak_count;

		Tp* ptr;

		control_block(const control_block&) = delete;
		control_block& operator=(const control_block&) = delete;

	public:
		control_block() = delete;

		control_block(Tp* other)
			: ptr(other), use_count(1), weak_count(0)
		{}

		virtual ~control_block()
		{}

		void add_ref_copy()
		{
			use_count++;
		}

		void destroy(std::true_type)
		{
			delete ptr;
		}

		void destroy(std::false_type)
		{
			delete ptr;

			if (0 == weak_count)
			{
				std::atomic_thread_fence(std::memory_order_acq_rel);


				delete this;
			}
		}

		void release() noexcept
		{

			if (1 == use_count--)
			{
				std::atomic_thread_fence(std::memory_order_acquire);



				destroy(std::bool_constant<std::conjunction_v<_Can_enable_shared<Tp>>>{});
			}
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
			weak_count++;
		}

		void weak_release() noexcept
		{
			if (1 == weak_count--)
			{
				std::atomic_thread_fence(std::memory_order_acq_rel);

				if (0 == use_count) {

					delete this;
				}
			}
		}
	};

	template <typename Tp>
	class shared_ptr {

	private:

		Tp* ptr;
		control_block<Tp>* ctr;
		mutable std::mutex sp_lock;
		
		friend class weak_ptr<Tp>;

		template<typename... Args>
		friend shared_ptr<Tp> make_shared(Args&&... _Args);

		void _Set_ptr_rep(Tp* other_ptr, control_block<Tp>* other_ctr)
		{
			ptr = other_ptr;
			ctr = other_ctr;
		}
	public:
		void _Set_ptr_rep_and_enable_shared(Tp* other_ptr, control_block<Tp>* other_ctr)
		{
			this->_Set_ptr_rep(other_ptr, other_ctr);
			_Enable_shared_from_this(*this, other_ptr);
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

		Tp* get_ptr()
		{
			std::lock_guard<std::mutex> get_ptr_guard(sp_lock);
			return ptr;
		}

		control_block<Tp>* get_ctr()
		{
			return ctr;
		}
		
		shared_ptr()
			: ptr(nullptr), ctr(nullptr)
		{}

		shared_ptr(nullptr_t)
			: shared_ptr()
		{}

		shared_ptr(Tp* other)
			: ptr(other), ctr(new control_block<Tp>(other))
		{}

		shared_ptr(const shared_ptr& other)
		{
			other.lock();

			other.ctr->add_ref_copy();
			ptr = other.ptr;
			ctr = other.ctr;

			other.unlock();
		}


		shared_ptr& operator=(nullptr_t)
		{
			lock();

			if (ctr) {
				ctr->release();
				ctr = nullptr;
				ptr = nullptr;
			}

			unlock();

			return *this;
		}

		shared_ptr& operator=(const shared_ptr& other)
		{
			
			lock();
			if (other.ptr == ptr) {
				unlock();
				return *this;
			}

			other.lock();

			other.ctr->add_ref_copy();
			control_block<Tp>* curr_ctr = ctr;
			ptr = other.ptr;
			ctr = other.ctr;
			
			other.unlock();

			if (curr_ctr) curr_ctr->release();
			unlock();

			return *this;
		}

		~shared_ptr()
		{

			if (ctr) 
				ctr->release();

		}

		Tp* get() const
		{
			return ptr;
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
			lock();

			if (ptr) {
				unlock();
				return true;
			}

			unlock();
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

			control_block<Tp>* curr_ctr = ctr;

			ptr = nullptr;
			ctr = nullptr;

			if (!curr_ctr->release())
				curr_ctr->unlock();
		}

		void swap(shared_ptr& other) // *** lock
		{
			
			Tp* emp_p;
			control_block<Tp>* emp_c;

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
				ctr->unlock();
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
		control_block<Tp>* new_ctr = new control_block<Tp>(new_Tp);

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
		control_block<Tp>* ctr;
		mutable std::mutex sp_lock;

		friend class shared_ptr<Tp>;

		friend class enable_shared_from_this<Tp>;

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

			control_block<Tp>* curr_ctr = ctr;
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

			control_block<Tp>* curr_ctr = ctr;
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

			control_block<Tp>* curr_ctr = ctr;

			ptr = nullptr;
			ctr = nullptr;

			if (!curr_ctr->weak_release())
				curr_ctr->unlock();
		}

		void swap(weak_ptr& other) {
			Tp* emp_p;
			control_block<Tp>* emp_c;

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

		friend void _Enable_shared_from_this1<Tp>(const shared_ptr<Tp>& _This, Tp* _Ptr, std::true_type);

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