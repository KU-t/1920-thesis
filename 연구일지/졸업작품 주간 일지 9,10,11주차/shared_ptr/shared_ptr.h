#include <atomic>

namespace LFSP {

	template<typename Tp>
	class shared_ptr;

	template<typename Tp>
	class weak_ptr;

	template<typename Tp>
	class ctr_block {
	public:
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

		int get_use_count()	// ****************************
		{
			int use_cnt;
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
				
				if(0 == use_count)
					delete this;
			}
		}

	private:
		//test
		template<typename Tp>
		friend class shared_ptr;

		//test
		template<typename Tp>
		friend class weak_ptr;


		ctr_block(const ctr_block&) = delete;
		ctr_block& operator=(const ctr_block&) = delete;

		std::atomic_int use_count;
		std::atomic_int	weak_count;

		Tp* ptr;
	};

	template <typename Tp>
	class shared_ptr {

	public:

		shared_ptr()
			: ptr(nullptr), ctr(nullptr)
		{
			std::cout << "기본 생성자 nullptr" << std::endl;
		}

		shared_ptr(nullptr_t)
			: shared_ptr()
		{}

		shared_ptr(Tp* other)
			: ptr(other), ctr(new ctr_block<Tp>(other))
		{
			std::cout << "객체ptr 복사 생성자" << std::endl;
		}

		shared_ptr(const shared_ptr& other)
		{
			std::cout << "shared_ptr shared_ptr 복사 생성자" << std::endl;

			if (!other.ptr) return;

			other.ctr->add_ref_copy();

			ptr = other.ptr;
			ctr = other.ctr;
		}

		shared_ptr(const weak_ptr<Tp>& other)
		{
			std::cout << "shared_ptr weak_ptr 복사 생성자" << std::endl;

			if (!other.ptr) return;

			other.ctr->add_ref_copy();

			ptr = other.ptr;
			ctr = other.ctr;
		}

		template<typename Tp>
		shared_ptr(const weak_ptr<Tp>& other) 
			: ptr(other.ptr), ctr(other.ctr)
		{
			
		}
		
		shared_ptr& operator=(const shared_ptr& other)
		{
			std::cout << "shared_ptr shared_ptr 대입 연산자" << std::endl;

			other.ctr->add_ref_copy();
			
			ctr->release();

			ptr = other.ptr;
			ctr = other.ctr;
			
			return *this;
		}

		shared_ptr& operator=(const weak_ptr<Tp>& other)
		{
			std::cout << "shared_ptr weak_ptr 대입 연산자" << std::endl;

			other.ctr->add_ref_copy();

			ctr->release();

			ptr = other.ptr;
			ctr = other.ctr;

			return *this;
		}
		
		~shared_ptr() 
		{
			if (!ptr)	return;
			
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

		int use_count()
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

		void info()	//**********test
		{
			if (ctr) {
				std::cout
					<< "strong : " << ctr->use_count
					<< " / weak : " << ctr->weak_count
					//<< " / " << *ptr
					<< std::endl;

			}

			else
				std::cout << "nullptr" << std::endl;
		}

	private:
		
		template<typename Yp> friend class shared_ptr;
		template<typename Tp> friend class weak_ptr;

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


	template<typename Tp>
	class weak_ptr
	{
	public:

		weak_ptr()
			: ptr(nullptr), ctr(nullptr)
		{
			std::cout << "기본 생성자 nullptr" << std::endl;
		}

		weak_ptr(nullptr_t)
			: weak_ptr()
		{}

		weak_ptr(const shared_ptr<Tp>& other) 
		{
			other.ctr->weak_add_ref();
			ctr = other.ctr;

			ptr = other.ptr;
		}

		weak_ptr(const weak_ptr<Tp>& other)
		{
			if (!other.ctr)	return;

			other.ctr->weak_add_ref();
			ctr = other.ctr;

			ptr = other.ptr;
		}

		~weak_ptr()
		{
			if (!ptr)	return;

			ctr->weak_release();
		}

		weak_ptr& operator=(const weak_ptr& other)
		{
			std::cout << "waek_ptr weak_ptr 대입 연산자" << std::endl;

			other.ctr->weak_add_ref();

			ctr->weak_release();

			ptr = other.ptr;
			ctr = other.ctr;

			return *this;
		}

		weak_ptr& operator=(const shared_ptr<Tp>& other)
		{
			std::cout << "waek_ptr shared_ptr 대입 연산자" << std::endl;

			other.ctr->weak_add_ref();

			ctr->weak_release();

			ptr = other.ptr;
			ctr = other.ctr;

			return *this;
		}

		shared_ptr<Tp> lock() const 
		{
			return shared_ptr<Tp>(*this);
		}

		int use_count()
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

		void info()	//**********test
		{
			if (ctr) {
				std::cout 
					<< "strong : " << ctr->use_count 
					<< " / weak : " << ctr->weak_count 
					<< " / " << *ptr
					<< std::endl;
				
			}

			else
				std::cout << "nullptr" << std::endl;
		}

	private:

		template<typename Yp> friend class weak_ptr;
		template<typename Tp> friend class shared_ptr;

		ctr_block<Tp>* ctr;
		Tp* ptr;
	};

	template<typename Tp, typename... Args>
	shared_ptr<Tp> make_shared(Args&&... _Args)
	{
		return shared_ptr<Tp>(new Tp(std::forward<Args>(_Args)...));
	}

	template<typename Tp>
	Tp atomic_load(const shared_ptr<Tp>* sptr)
	{
		return sptr->get();
	}

	template<typename Tp>
	Tp atomic_load(const weak_ptr<Tp>* wptr)
	{
		return shared_ptr<Tp>(wptr).get();
	}

	template<typename Tp>
	Tp atomic_store(const shared_ptr<Tp>* sptr, Tp ptr)
	{
		sptr->ptr = ptr;

		return sptr->get();
	}
	
	template<typename Tp>
	Tp atomic_store(const weak_ptr<Tp>* wptr, Tp ptr)
	{
		wptr->ptr = ptr;

		return shared_ptr<Tp>(wptr).get();
	}

	template<typename Tp>
	class enable_shared_from_this
	{
	public:
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
			:Wptr()
		{}

		enable_shared_from_this(const enable_shared_from_this&)
			:Wptr()
		{}

		enable_shared_from_this& operator=(const enable_shared_from_this&) 
		{	
			return (*this);
		}

		~enable_shared_from_this() = default;

	private:
		weak_ptr<Tp> Wptr;
	};
}

