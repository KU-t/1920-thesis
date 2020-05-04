#pragma once
#include <Windows.h>
#include <atomic>
#include <mutex>

namespace LFSP {

	template<typename Tp>
	class shared_ptr;

	template<typename Tp>
	class weak_ptr;

	template<typename Tp>
	class enable_shared_from_this;

	template<typename Tp>
	class ctr_block {
	public:
		ctr_block() = delete;

		ctr_block(Tp* other)
			: ptr(other), use_count(1), weak_count(0)
		{}

		virtual ~ctr_block()
		{}

		int get_weak_count()
		{
			return weak_count;
		}

		void add_ref_copy()
		{
			use_count++;
		}

		void  release() noexcept
		{
			if (1 == use_count--)
			{
				std::atomic_thread_fence(std::memory_order_acquire);
				delete ptr;

				if (weak_count == 0) {
					//DebugBreak();
					std::cout << "THIS : " << this << std::endl;
					std::cout << "weak_count : " << weak_count << std::endl;
					std::cout << "PTR : " << this->ptr << std::endl;
					std::cout << "use_count : " << use_count << std::endl;
					delete this;
				}
			}
		}

		std::atomic_int get_use_count()	// ****************************
		{
			std::atomic_int use_cnt;
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
			std::cout << "Weak Release Called, on : " << this << "  weak_count = " << weak_count << std::endl;
			if (1 == weak_count--)
			{
				std::atomic_thread_fence(std::memory_order_acq_rel);

				if (0 == use_count)
					delete this;
			}
		}

		void lock() { 
			ml.lock(); 
		}

		void unlock() { 
			ml.unlock(); 
		}

	private:
		
		ctr_block(const ctr_block&) = delete;
		ctr_block& operator=(const ctr_block&) = delete;

		std::atomic_int use_count;
		std::atomic_int	weak_count;

		Tp* ptr;

		std::mutex ml;
	};

	template <typename Tp>
	class shared_ptr {

	public:

		shared_ptr()
			: ptr(nullptr), ctr(nullptr)
		{}

		shared_ptr(nullptr_t)
			: shared_ptr()
		{}

		shared_ptr(Tp* other)
			: ptr(other), ctr(new ctr_block<Tp>(other))
		{
			//_Enable_shared_from_this(*this, ctr);
		}

		shared_ptr(const shared_ptr& other)
		{
			if (!other.ctr) return;

			other.ctr->add_ref_copy();

			other.ctr->lock();
			ptr = other.ptr;
			ctr = other.ctr;
			other.ctr->unlock();
		}

		shared_ptr(const weak_ptr<Tp>& other)
		{
			if (!other.ctr) return;

			other.ctr->weak_add_ref();

			other.ctr->lock();
			ptr = other.ptr;
			ctr = other.ctr;
			other.ctr->unlock();
		}


		shared_ptr& operator=(const shared_ptr& other)
		{
			if (!other.ctr)
				return *this;

			other.ctr->add_ref_copy();

			if(ctr)
				ctr->release();

			other.ctr->lock();
			ptr = other.ptr;
			ctr = other.ctr;
			other.ctr->unlock();

			return *this;
		}

		~shared_ptr()
		{
			if (!ctr)	return;

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

		Tp* operator->()
		{
			return get();
		}

		operator bool()
		{
			if (ptr)	return true;
			return false;
		}

		std::atomic_int use_count()
		{
			return ctr->get_use_count();
		}

		void reset()
		{
			if (!ptr)	return;

			ctr->release();
			ptr = nullptr;
			ctr = nullptr;
		}

		void swap(shared_ptr& other) {
			if (!other.ctr)
				return;
			
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

	private:

		template<typename Tp>
		void _Set_ptr_rep(Tp* other_ptr, ctr_block<Tp>* other_ctr)
		{
			ptr = other_ptr;
			ctr = other_ctr;
		}

		template<typename Tp>
		void _Set_ptr_rep_and_enable_shared(Tp* other_ptr, ctr_block<Tp>* other_ctr)
		{
			this->_Set_ptr_rep(other_ptr, other_ctr);
			_Enable_shared_from_this(*this, other_ptr);
		}

		template<typename Tp>
		friend class weak_ptr;


		template<typename Tp, typename... Args>
		friend shared_ptr<Tp> make_shared(Args&&... _Args);

		ctr_block<Tp>* ctr;
		Tp* ptr;
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
	{	// write contained pointer to stream
		return (_Out << _Px.get());
	}

	template<typename Tp, typename... Args>
	shared_ptr<Tp> make_shared(Args&&... _Args)
	{
		// Create ptr
		Tp* new_Tp = new Tp(std::forward<Args>(_Args)...);

		// Create ctr_block
		// init : ptr, use_count, weak_count
		ctr_block<Tp>* new_ctr = new ctr_block<Tp>(new_Tp);


		// Create shared_ptr
		shared_ptr<Tp> _Ret;
		_Ret._Set_ptr_rep_and_enable_shared(new_Tp, new_ctr);

		return (_Ret);
	}

	template<typename Tp, typename = void>
	struct _Can_enable_shared : std::false_type
	{	// detect unambiguous and accessible inheritance from enable_shared_from_this
	};

	template<typename Tp>
	struct _Can_enable_shared<Tp, std::void_t<typename Tp::_Esft_type>>
		: std::is_convertible<std::remove_cv_t<Tp> *, typename Tp::_Esft_type *>::type
	{	// is_convertible is necessary to verify unambiguous inheritance
	};

	template<typename Tp>
	void _Enable_shared_from_this1(const shared_ptr<Tp>& _This, Tp* _Ptr, std::true_type)
	{	// enable shared_from_this

		enable_shared_from_this<Tp>* enable_shared_from_this_base
			= reinterpret_cast<enable_shared_from_this<Tp>*>(_Ptr);

		enable_shared_from_this_base->Wptr = _This;

	}

	template<typename Tp>
	void _Enable_shared_from_this1(const shared_ptr<Tp>&, Tp*, std::false_type)
	{	// don't enable shared_from_this
	}

	template<typename Tp>
	void _Enable_shared_from_this(const shared_ptr<Tp>& _This, Tp * _Ptr)
	{	// possibly enable shared_from_this
		_Enable_shared_from_this1(_This, _Ptr,
			std::bool_constant<std::conjunction_v<_Can_enable_shared<Tp>>>{});
	}

	template<typename Tp>
	class weak_ptr
	{
	public:
		ctr_block<Tp> * get_control_block()
		{
			return ctr;
		}

		int get_weak_counter()
		{
			return ctr->get_weak_count();
		}

		weak_ptr()
			: ptr(nullptr), ctr(nullptr)
		{
			std::cout << "Creating Weak Pointer from nothing!\n";
		}

		weak_ptr(nullptr_t)
			: weak_ptr()
		{}

		weak_ptr(const shared_ptr<Tp>& other)
		{
			if (!other.ctr)
				return;

			//DebugBreak();
			std::cout << "Creating Weak Pointer from shared pointer!\n";

			other.ctr->weak_add_ref();

			other.ctr->lock();
			ptr = other.ptr;
			ctr = other.ctr;
			other.ctr->unlock();
		}

		weak_ptr(const weak_ptr<Tp>& other)
		{
			if (!other.ctr)	return;

			std::cout << "Creating Weak Pointer from Weak Pointer!\n";

			other.ctr->weak_add_ref();

			other.ctr->lock();
			ptr = other.ptr;
			ctr = other.ctr;
			other.ctr->unlock();
		}

		~weak_ptr()
		{
			std::cout << "Destruct Weak_Ptr : " << this << std::endl;
			if (!ptr)	return;

			ctr->weak_release();
		}

		weak_ptr& operator=(const shared_ptr<Tp>& other)
		{
			if (!other.ctr)
				return *this;
			other.ctr->weak_add_ref();

			if(ctr)
				ctr->weak_release();

			other.ctr->lock();
			ptr = other.ptr;
			ctr = other.ctr;
			other.ctr->unlock();

			return *this;
		}

		weak_ptr& operator=(const weak_ptr& other)
		{
			if (!other.ctr)
				return *this; 
			
			other.ctr->weak_add_ref();

			if(ctr)
				ctr->weak_release();

			other.ctr->lock();
			ptr = other.ptr;
			ctr = other.ctr;
			other.ctr->unlock();

			return *this;
		}

		shared_ptr<Tp> lock() const
		{
			if (ptr)
				return shared_ptr<Tp>(*this);

			else
				return nullptr;
		}

		void reset()
		{
			if (!ptr)	return;

			if(ctr)
				ctr->weak_release();
			ptr = nullptr;
			ctr = nullptr;
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

	private:

		template<typename Tp> 
		friend class shared_ptr;

		template<typename Tp> 
		friend class enable_shared_from_this;

		ctr_block<Tp>* ctr;
		Tp* ptr;
	};


	template<typename Tp>
	class enable_shared_from_this
	{
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

	private:
		template<typename Tp>
		friend void _Enable_shared_from_this1(const shared_ptr<Tp>& _This, Tp* _Ptr, std::true_type);


		weak_ptr<Tp> Wptr;
	};
}

