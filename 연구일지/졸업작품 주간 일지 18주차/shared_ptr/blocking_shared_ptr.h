#pragma once

//#define RELEASE_COUT_DEBUGING_TEST
//#define MUTEX_LOCK_TEST

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
	{	// detect unambiguous and accessible inheritance from enable_shared_from_this
	};

	template<typename Tp>
	struct _Can_enable_shared<Tp, std::void_t<typename Tp::_Esft_type>>
		: std::is_convertible<std::remove_cv_t<Tp> *, typename Tp::_Esft_type *>::type
	{	// is_convertible is necessary to verify unambiguous inheritance
	};

	template<typename Tp>
	class ctr_block {
	public:

		// **************************
#ifdef RELEASE_COUT_DEBUGING_TEST

		int get_usecount()
		{
			return use_count;
		}

		int get_weakcount()
		{
			return weak_count;
		}

#endif // RELEASE_COUT_DEBUGING_TEST

		ctr_block() = delete;

		ctr_block(Tp* other)
			: ptr(other), use_count(1), weak_count(0)
		{}

		virtual ~ctr_block()
		{}

		void add_ref_copy()
		{
			use_count++;
		}

		bool destroy(std::true_type)
		{
#ifdef RELEASE_COUT_DEBUGING_TEST
			std::cout << "[Control Block Destroy true_type]" << std::endl;
#endif // RELEASE_COUT_DEBUGING_TEST
			
			delete ptr;

			ptr = nullptr;

			return false;
		}

		bool destroy(std::false_type)
		{
#ifdef RELEASE_COUT_DEBUGING_TEST
			std::cout << "[Control Block Destroy false_type]" << std::endl;
#endif // RELEASE_COUT_DEBUGING_TEST
			delete ptr;

			ptr = nullptr;

			if (0 == weak_count)
			{
				std::atomic_thread_fence(std::memory_order_acq_rel);
#ifdef RELEASE_COUT_DEBUGING_TEST
				std::cout << "[Control Block release Delete this] : " << this << std::endl;
#endif // RELEASE_COUT_DEBUGING_TEST
				
				ml.unlock();
				delete this;

				return true;
			}

			return false;
		}

		bool release() noexcept
		{
#ifdef RELEASE_COUT_DEBUGING_TEST
			std::cout << "[ Control_block release ] :" << this << std::endl;
			std::cout << "[ Control_block release ] Use_count : " << use_count << std::endl;
			std::cout << "[ Control_block release ] Weak_count : " << weak_count << std::endl;
#endif // RELEASE_COUT_DEBUGING_TEST

			if (1 == use_count--)
			{
				std::atomic_thread_fence(std::memory_order_acquire);

#ifdef RELEASE_COUT_DEBUGING_TEST
				std::cout << "[ Control_block release::delete ptr ] :" << ptr << std::endl;
#endif // RELEASE_COUT_DEBUGING_TEST

				return destroy(std::bool_constant<std::conjunction_v<_Can_enable_shared<Tp>>>{});
			}

			return false;
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

		bool weak_release() noexcept
		{
#ifdef RELEASE_COUT_DEBUGING_TEST
			std::cout << "[ Control_block weak_release ] :" << this << std::endl;
			std::cout << "[ Control_block weak_release ] Use_count : " << use_count << std::endl;
			std::cout << "[ Control_block weak_release ] Weak_count : " << weak_count << std::endl;
#endif // RELEASE_COUT_DEBUGING_TEST


			if (1 == weak_count--)
			{
				std::atomic_thread_fence(std::memory_order_acq_rel);

				if (0 == use_count) {
#ifdef RELEASE_COUT_DEBUGING_TEST
					std::cout << "[Control Block weak_release Delete this] : " << this << std::endl;
#endif // RELEASE_COUT_DEBUGING_TEST

					ml.unlock();
					delete this;

					return true;
				}
			}

			return false;
		}

		void lock() {
#ifdef MUTEX_LOCK_TEST
			std::cout << "Try lock() : " << this << std::endl;
#endif // MUTEX_LOCK_TEST
			ml.lock();
		}

		void unlock() {
#ifdef MUTEX_LOCK_TEST
			std::cout << "Try unlock() : " << this << std::endl;
#endif // MUTEX_LOCK_TEST
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

		// ********************
#ifdef RELEASE_COUT_DEBUGING_TEST
		int get_usecount()
		{
			return ctr->get_usecount();
		}

		int get_weakcount()
		{
			return ctr->get_weakcount();
		}

#endif // RELEASE_COUT_DEBUGING_TEST

		Tp* get_ptr()
		{
			return ptr;
		}

		ctr_block<Tp>* get_ctr()
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
			: ptr(other), ctr(new ctr_block<Tp>(other))
		{}

		shared_ptr(const shared_ptr& other)
		{
			other.ctr->lock();

			other.ctr->add_ref_copy();
			ptr = other.ptr;
			ctr = other.ctr;

			other.ctr->unlock();
		}

		shared_ptr(const weak_ptr<Tp>& other)
		{
			other.ctr->lock();

			other.ctr->add_ref_copy();
			ptr = other.ptr;
			ctr = other.ctr;

			other.ctr->unlock();
		}

		shared_ptr& operator=(nullptr_t)
		{
			if (ctr) {
				ctr->lock();

				ctr_block<Tp>* curr_ctr = ctr;

				ctr = nullptr;
				ptr = nullptr;

				if (!curr_ctr->release())
					curr_ctr->unlock();
			}

			return *this;
		}

		shared_ptr& operator=(const shared_ptr& other)
		{
			if (other.ptr == ptr)
				return *this;

			if (ctr)
				ctr->lock();

			if (other.ctr)
				other.ctr->lock();

			else {
				if (ctr) {
					ctr_block<Tp>* curr_ctr = ctr;
				
					ctr = nullptr;
					ptr = nullptr;

					if(!curr_ctr->release())
						curr_ctr->unlock();
				}
				return *this;
			}

			other.ctr->add_ref_copy();

			ctr_block<Tp>* curr_ctr = ctr;

			ptr = other.ptr;
			ctr = other.ctr;

			other.ctr->unlock();

			if (curr_ctr) {
				if(!curr_ctr->release())
					curr_ctr->unlock();
			}

			return *this;
		}

		~shared_ptr()
		{
			if (!ctr)	return;

#ifdef RELEASE_COUT_DEBUGING_TEST
			std::cout << "[ shared_ptr Destruct ] : " << this << std::endl;
			std::cout << "[ shared_ptr Destruct ]ptr : " << ptr << std::endl;
			std::cout << "[ shared_ptr Destruct ]ctr : " << ctr << std::endl;
			std::cout << "[ shared_ptr Destruct ]Use_count : " << ctr->get_usecount() << std::endl;
			std::cout << "[ shared_ptr Destruct ]Weak_count : " << ctr->get_weakcount() << std::endl;
#endif // RELEASE_COUT_DEBUGING_TEST

			ctr->lock();
			if (!ctr->release())
				ctr->unlock();
		}

		Tp* blocking_get() const
		{
			ctr->lock();
			Tp* Ret;
			Ret = ptr;
			ctr->unlock();

			return Ret;
		}

		Tp* get() const
		{
			return blocking_get();
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
			if (ptr)	
				return true;

			return false;
		}

		int use_count()
		{
			return ctr->get_use_count();
		}

		void reset()	
		{
			if (!ctr)	return;

			ctr->lock();

			ctr_block<Tp>* curr_ctr = ctr;

			ptr = nullptr;
			ctr = nullptr;

			if (!curr_ctr->release())
				curr_ctr->unlock();
		}

		void swap(shared_ptr& other) // *** lock
		{
			
			Tp* emp_p;
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
#ifdef RELEASE_COUT_DEBUGING_TEST
		std::cout << "function : make_shared() \n";
		std::cout << "function : make_shared::Create new Tp : ";
#endif // RELEASE_COUT_DEBUGING_TEST
		Tp* new_Tp = new Tp(std::forward<Args>(_Args)...);
#ifdef RELEASE_COUT_DEBUGING_TEST
		std::cout << new_Tp << std::endl;
#endif // RELEASE_COUT_DEBUGING_TEST

		// Create ctr_block
		// init : ptr, use_count, weak_count
#ifdef RELEASE_COUT_DEBUGING_TEST
		std::cout << "function : make_shared::Create new_ctr : ";
#endif // RELEASE_COUT_DEBUGING_TEST
		ctr_block<Tp>* new_ctr = new ctr_block<Tp>(new_Tp);
#ifdef RELEASE_COUT_DEBUGING_TEST
		std::cout << new_ctr << std::endl;
#endif // RELEASE_COUT_DEBUGING_TEST

		// Create shared_ptr
#ifdef RELEASE_COUT_DEBUGING_TEST
		std::cout << "function : make_shared::Create shared_ptr \n";
#endif // RELEASE_COUT_DEBUGING_TEST
		shared_ptr<Tp> _Ret;
		_Ret._Set_ptr_rep_and_enable_shared(new_Tp, new_ctr);

#ifdef RELEASE_COUT_DEBUGING_TEST
		std::cout << "function : make_shared::(return)shared_ptr Use_count : " << _Ret.get_usecount() << std::endl;
		std::cout << "function : make_shared::(return)shared_ptr Weak_count : " << _Ret.get_weakcount() << std::endl;
		std::cout << "function : make_shared::(return)shared_ptr Ptr: " << _Ret.get_ptr() << std::endl;
		std::cout << "function : make_shared::(return)shared_ptr Ctr : " << _Ret.get_ctr() << std::endl;
#endif // RELEASE_COUT_DEBUGING_TEST

		return (_Ret);
	}

	template<typename Tp>
	void _Enable_shared_from_this1(const shared_ptr<Tp>& _This, Tp* _Ptr, std::true_type)
	{	// enable shared_from_this

		enable_shared_from_this<Tp>* enable_shared_from_this_base
			= reinterpret_cast<enable_shared_from_this<Tp>*>(_Ptr);
		//DebugBreak();
		enable_shared_from_this_base->Wptr = _This;

#ifdef RELEASE_COUT_DEBUGING_TEST
		std::cout << "enable_shared_from_this::Weak_ptr control_block : " << enable_shared_from_this_base->Wptr.get_ctr() << std::endl;
#endif // RELEASE_COUT_DEBUGING_TEST
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

		// ********************

#ifdef RELEASE_COUT_DEBUGING_TEST
		int get_usecount()
		{
			return ctr->get_usecount();
		}

		int get_weakcount()
		{
			return ctr->get_weakcount();
		}

		ctr_block<Tp>* get_ctr()
		{
			return ctr;
		}
#endif // RELEASE_COUT_DEBUGING_TEST

		weak_ptr()
			: ptr(nullptr), ctr(nullptr)
		{
#ifdef RELEASE_COUT_DEBUGING_TEST
			std::cout << "[ Create Weak_ptr nullptr ] : " << this;
#endif // RELEASE_COUT_DEBUGING_TEST
		}

		weak_ptr(nullptr_t)
			: weak_ptr()
		{}

		weak_ptr(const shared_ptr<Tp>& other)
		{
#ifdef RELEASE_COUT_DEBUGING_TEST
			std::cout << "[ Create Weak_ptr from shared_ptr ] : " << this;
#endif // RELEASE_COUT_DEBUGING_TEST

			/*if (!other.ctr) {
				weak_ptr();
				return;
			}*/

			other.ctr->lock();
			other.ctr->weak_add_ref();

			ptr = other.ptr;
			ctr = other.ctr;
			other.ctr->unlock();
		}

		weak_ptr(const weak_ptr<Tp>& other)
		{
#ifdef RELEASE_COUT_DEBUGING_TEST
			std::cout << "[ Create Weak_ptr from weak_ptr ]\n";
#endif // RELEASE_COUT_DEBUGING_TEST

			/*if (!other.ctr) {
				weak_ptr();
				return;
			}*/

			other.ctr->lock();
			other.ctr->weak_add_ref();

			ptr = other.ptr;
			ctr = other.ctr;
			other.ctr->unlock();
		}

		~weak_ptr()
		{
#ifdef RELEASE_COUT_DEBUGING_TEST
			std::cout << "[ Weak_ptr Destruct ] : " << this << std::endl;
			std::cout << "[ Weak_ptr Destruct ]ptr : " << ptr << std::endl;
			std::cout << "[ Weak_ptr Destruct ]ctr : " << ctr << std::endl;
			std::cout << "[ Weak_ptr Destruct ]Use_count : " << ctr->get_usecount() << std::endl;
			std::cout << "[ Weak_ptr Destruct ]Weak_count : " << ctr->get_weakcount() << std::endl;
#endif // RELEASE_COUT_DEBUGING_TEST

			ctr->lock();
			if (!ctr->weak_release())
				ctr->unlock();
		}

		weak_ptr& operator=(const shared_ptr<Tp>& other)
		{
			if (other.ptr == ptr)
				return *this;

			if (ctr)
				ctr->lock();

			if (other.ctr)
				other.ctr->lock();

			else {
				if (ctr) {
					ctr_block<Tp>* pred_ctr = ctr;

					ctr = nullptr;
					ptr = nullptr;

					if (!pred_ctr->weak_release())
						pred_ctr->unlock();
				}
				return *this;
			}

			other.ctr->weak_add_ref();

			ctr_block<Tp>* curr_ctr = ctr;

			ptr = other.ptr;
			ctr = other.ctr;

			other.ctr->unlock();

			if (curr_ctr) {
				if (!curr_ctr->weak_release())
					curr_ctr->unlock();
			}

			return *this;
		}

		weak_ptr& operator=(const weak_ptr& other)
		{
			if (other.ptr == ptr)
				return *this;

			if (ctr)
				ctr->lock();

			if (other.ctr)
				other.ctr->lock();

			else {
				if (ctr) {
					ctr_block<Tp>* pred_ctr = ctr;

					ctr = nullptr;
					ptr = nullptr;

					if (!pred_ctr->weak_release())
						pred_ctr->unlock();
				}
				return *this;
			}

			other.ctr->weak_add_ref();

			ctr_block<Tp>* curr_ctr = ctr;

			ptr = other.ptr;
			ctr = other.ctr;

			other.ctr->unlock();

			if (curr_ctr) {
				if (!curr_ctr->weak_release())
					curr_ctr->unlock();
			}

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

	private:

		template<typename Tp>
		friend class shared_ptr;

		template<typename Tp>
		friend class enable_shared_from_this;

		ctr_block<Tp>* ctr;
		Tp* ptr;
		mutex sp_lock;
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
		{
#ifdef RELEASE_COUT_DEBUGING_TEST
			std::cout << "[ Create enable_shared_from_this ]";
#endif // RELEASE_COUT_DEBUGING_TEST
		}

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

	// ****************************

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