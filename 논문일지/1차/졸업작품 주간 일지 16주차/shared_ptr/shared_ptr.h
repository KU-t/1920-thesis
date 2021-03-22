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
			: ptr(other), use_count(0), weak_count(0)
		{}

		virtual ~ctr_block()
		{}

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

				if (weak_count == 0)
					delete this;
			}
		}

		std::atomic_int get_use_count()	// ****************************
		{
			std::atomic_int use_cnt;
			do
			{
				use_cnt = use_count;
			} while (!atomic_compare_exchange_strong(&use_count, &use_cnt, use_cnt));

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

				if (0 == use_count)
					delete this;
			}
		}

		void lock() { ml.lock(); }

		void unlock() { ml.unlock(); }

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

		shared_ptr(const shared_ptr& other)
		{
			
			if (!other.ptr) return;

			other.ctr->add_ref_copy();

			other.ctr->lock();
			ptr = other.ptr;
			ctr = other.ctr;
			other.ctr->unlock();
		}

		shared_ptr(const weak_ptr<Tp>& other)
		{

			if (!other.ptr) return;

			other.ctr->add_ref_copy();

			other.ctr->lock();
			ptr = other.ptr;
			ctr = other.ctr;
			other.ctr->unlock();
		}

		~shared_ptr()
		{
			if (!ptr)	return;

			ctr->release();
		}

		shared_ptr& operator=(const shared_ptr& other)
		{
			other.ctr->add_ref_copy();

			if (ctr)
				ctr->release();

			other.ctr->lock();
			ptr = other.ptr;
			ctr = other.ctr;
			other.ctr->unlock();

			return *this;
		}

		Tp* get()
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

		std::atomic_int use_count()
		{
			return ctr->get_use_count();
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

		template<typename Yp>
		friend class shared_ptr;

		template<typename Tp>
		friend class weak_ptr;


		template<typename Tp, typename... Args>
		friend shared_ptr<Tp> make_shared(Args&&... _Args);

		ctr_block<Tp>* ctr;
		Tp* ptr;
	};

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

		// 어떻게 넘겨야할까 1
		/*enable_shared_from_this<Tp>* enable_shared_from_this_base
			= dynamic_cast<enable_shared_from_this<Tp>*>(_Ptr);
		
		if(enable_shared_from_this_base)
			enable_shared_from_this_base->Wptr = _This;*/

		// 어떻게 넘겨야할까 2
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

		weak_ptr()
			: ptr(nullptr), ctr(nullptr)
		{}

		weak_ptr(const shared_ptr<Tp>& other)
		{
			other.ctr->weak_add_ref();

			other.ctr->lock();
			ptr = other.ptr;
			ctr = other.ctr;
			other.ctr->unlock();
		}

	private:

		std::atomic_int use_count()
		{
			return ctr->use_count();
		}

		bool expired()
		{
			return (this->use_count() == 0);
		}

		template<typename Yp>
		friend class weak_ptr;

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