
#ifndef _SHARED_PTR_BASE_H
#define _SHARED_PTR_BASE_H 1

#include <atomic>

namespace LFSP {

	inline void bad_pointer() {
		printf("bad_pointer");
		system("pause");
	}

	class __counted_base 
	{
	public:
		__counted_base() noexcept
			: _M_use_count(1),
			_M_weak_count(1)
		{ }

		virtual	~__counted_base() noexcept
		{ }

		virtual void _M_dispose() noexcept = 0;

		virtual void _M_destroy() noexcept
		{
			delete this;
		}

		void _M_add_ref_copy()
		{
			_M_use_count++;
		}

		void _M_add_ref_lock() 
		{
			int __count = _M_get_use_count();
			
			do 
			{
				if (__count == 0)	bad_pointer();
				
			} while (!atomic_compare_exchange_strong(&_M_use_count, &__count, __count + 1));
		}

		bool _M_add_ref_lock_nothrow()
		{
			int __count = _M_get_use_count();
			
			do 
			{
				if (__count == 0)	return false;
				
			} while (!atomic_compare_exchange_strong(&_M_use_count, &__count, __count + 1));

			return true;
		}

		void _M_release() noexcept
		{
			if (1 == _M_use_count--)
			{
				_M_dispose();
				std::atomic_thread_fence(std::memory_order_acquire);

				if (1 == _M_weak_count--)
				{
					_M_destroy();
				}
			}
		}

		void _M_weak_add_ref() noexcept
		{
			_M_weak_count++;
		}

		void _M_weak_release() noexcept							
		{
			if (1 == _M_weak_count--) 
			{
				std::atomic_thread_fence(std::memory_order_acq_rel);
				_M_destroy();
			}
		}

		long _M_get_use_count() const noexcept
		{
			return _M_use_count.load(std::memory_order_relaxed);
		}

	private:
		__counted_base(const __counted_base &) = delete;

		__counted_base& operator=(const __counted_base &) = delete;

		std::atomic_int _M_use_count;
		std::atomic_int _M_weak_count;
	};



	template<typename _Tp>
	class __shared_ptr;

	template<typename _Tp>
	class __weak_ptr;

	//template<typename _Tp>
	//class __enable_shared_from_this;

	template<typename _Tp>
	class shared_ptr;

	template<typename _Tp>
	class weak_ptr;

	//template<typename _Tp>
	//struct owner_less;

	//template<typename _Tp>
	//class enable_shared_from_this;

	class __weak_count;

	class __shared_count;



	template<typename _Ptr>
	class __counted_ptr final : public __counted_base
	{
	public:
		explicit __counted_ptr(_Ptr __p) noexcept
			: _M_ptr(__p) 
		{ }

		virtual void _M_dispose() noexcept
		{
			delete _M_ptr;
		}

		virtual void _M_destroy() noexcept
		{
			delete this;
		}


		__counted_ptr(const __counted_ptr&) = delete;
		__counted_ptr& operator=(const __counted_ptr&) = delete;

	private:
		_Ptr             _M_ptr;
	};


	template<>
	inline void __counted_ptr<nullptr_t>::_M_dispose() noexcept
	{ }

	class __shared_count 
	{
	public:
		constexpr __shared_count() noexcept 
			: _M_pi(0)
		{ }

		template<typename _Ptr>
		explicit __shared_count(_Ptr __p) 
			: _M_pi(0)
		{

			try 
			{
				_M_pi = new __counted_ptr<_Ptr>(__p);
			}

			catch (...) 
			{
				delete __p;
				bad_pointer();
			}
		}
		
		inline explicit __shared_count(const __weak_count& __r)
			: _M_pi(__r._M_pi)
		{
			if (_M_pi != nullptr)
				_M_pi->_M_add_ref_lock();

			else
				bad_pointer();
		}

		inline explicit __shared_count(const __weak_count& __r, std::nothrow_t)
			: _M_pi(__r._M_pi)
		{
			if (_M_pi != nullptr)
			{
				if (!_M_pi->_M_add_ref_lock_nothrow())
					_M_pi = nullptr;
			}
		}

		~__shared_count() noexcept
		{
			if (_M_pi != nullptr) 
				_M_pi->_M_release();
		}

		__shared_count(const __shared_count& __r) noexcept
			: _M_pi(__r._M_pi)
		{
			if (_M_pi != 0)
				_M_pi->_M_add_ref_copy();
		}

		__shared_count& operator=(const __shared_count& __r) noexcept
		{
			__counted_base* __tmp = __r._M_pi;
			
			if (__tmp != _M_pi)
			{
				if (__tmp != 0)
					__tmp->_M_add_ref_copy();

				if (_M_pi != 0)
					_M_pi->_M_release();

				_M_pi = __tmp;
			}
			
			return *this;
		}

		void _M_swap(__shared_count& __r) noexcept
		{
			__counted_base* __tmp = __r._M_pi;
			__r._M_pi = _M_pi;
			_M_pi = __tmp;
		}

		long _M_get_use_count() const noexcept
		{
			return _M_pi != 0 ? _M_pi->_M_get_use_count() : 0;
		}

		/*bool _M_less(const __shared_count& __rhs) const noexcept
		{
			return std::less<__counted_base*>()(this->_M_pi, __rhs._M_pi);
		}

		bool _M_less(const __weak_count& __rhs) const noexcept
		{
			return std::less<__counted_base*>()(this->_M_pi, __rhs._M_pi);
		}*/

		friend inline bool operator==(const __shared_count& __a, const __shared_count& __b) noexcept
		{
			return __a._M_pi == __b._M_pi;
		}

	private:
		friend class __weak_count;

		__counted_base*  _M_pi;
	};

	class __weak_count
	{
	public:
		constexpr __weak_count() noexcept
			: _M_pi(nullptr)
		{ }

		__weak_count(const __shared_count& __r) noexcept
			: _M_pi(__r._M_pi)
		{
			if (_M_pi != nullptr)
				_M_pi->_M_weak_add_ref();
		}

		__weak_count(const __weak_count& __r) noexcept
			: _M_pi(__r._M_pi)
		{
			if (_M_pi != nullptr)
				_M_pi->_M_weak_add_ref();
		}

		__weak_count(__weak_count&& __r) noexcept
			: _M_pi(__r._M_pi)
		{
			__r._M_pi = nullptr;
		}

		~__weak_count() noexcept
		{
			if (_M_pi != nullptr)
				_M_pi->_M_weak_release();
		}

		__weak_count& operator=(const __shared_count& __r) noexcept
		{
			__counted_base* __tmp = __r._M_pi;

			if (__tmp != nullptr)
				__tmp->_M_weak_add_ref();

			if (_M_pi != nullptr)
				_M_pi->_M_weak_release();

			_M_pi = __tmp;

			return *this;
		}

		__weak_count& operator=(const __weak_count& __r) noexcept
		{
			__counted_base* __tmp = __r._M_pi;

			if (__tmp != nullptr)
				__tmp->_M_weak_add_ref();

			if (_M_pi != nullptr)
				_M_pi->_M_weak_release();

			_M_pi = __tmp;

			return *this;
		}

		__weak_count& operator=(__weak_count&& __r) noexcept
		{
			if (_M_pi != nullptr)
				_M_pi->_M_weak_release();

			_M_pi = __r._M_pi;
			__r._M_pi = nullptr;

			return *this;
		}

		void _M_swap(__weak_count& __r) noexcept
		{
			__counted_base* __tmp = __r._M_pi;
			__r._M_pi = _M_pi;
			_M_pi = __tmp;
		}

		long _M_get_use_count() const noexcept
		{
			return _M_pi != nullptr ? _M_pi->_M_get_use_count() : 0;
		}

		/*bool _M_less(const __weak_count& __rhs) const noexcept
		{
			return std::less<__counted_base*>()(this->_M_pi, __rhs._M_pi);
		}

		bool _M_less(const __shared_count& __rhs) const noexcept
		{
			return std::less<__counted_base*>()(this->_M_pi, __rhs._M_pi);
		}*/

		friend inline bool operator==(const __weak_count& __a, const __weak_count& __b) noexcept
		{
			return __a._M_pi == __b._M_pi;
		}

	private:
		friend class __shared_count;

		__counted_base*  _M_pi;
	};

	template<typename _Tp>
	class __shared_ptr
	{
	public:
		using element_type = typename _Tp;

	private:
		element_type* _M_get() const noexcept
		{
			return static_cast<const __shared_ptr<_Tp>*>(this)->get();
		}

	public:
		element_type& operator*() const noexcept
		{
			return *_M_get();
		}

		element_type* operator->() const noexcept
		{
			return _M_get();
		}

	public:
		constexpr __shared_ptr() noexcept
			: _M_ptr(0), _M_refcount()
		{ }

		template<typename _Yp>
		__shared_ptr(const __shared_ptr<_Yp>& __r, element_type* __p) noexcept
			: _M_ptr(__p), _M_refcount(__r._M_refcount) // never throws
		{ }

		// Aliasing constructor
		template<typename _Yp>
		__shared_ptr(__shared_ptr<_Yp>&& __r, element_type* __p) noexcept
			: _M_ptr(__p), _M_refcount()
		{
			_M_refcount._M_swap(__r._M_refcount);
			__r._M_ptr = 0;
		}

		__shared_ptr(const __shared_ptr&) noexcept = default;
		__shared_ptr& operator=(const __shared_ptr&) noexcept = default;

		~__shared_ptr() = default;

		template<typename _Yp>
		__shared_ptr(const __shared_ptr<_Yp>& __r) noexcept
			: _M_ptr(__r._M_ptr), _M_refcount(__r._M_refcount)
		{ }

		__shared_ptr(__shared_ptr&& __r) noexcept
			: _M_ptr(__r._M_ptr), _M_refcount()
		{
			_M_refcount._M_swap(__r._M_refcount);
			__r._M_ptr = 0;
		}

		template<typename _Yp>
		__shared_ptr(__shared_ptr<_Yp>&& __r) noexcept
			: _M_ptr(__r._M_ptr), _M_refcount()
		{
			_M_refcount._M_swap(__r._M_refcount);
			__r._M_ptr = 0;
		}

		template<typename _Yp>
		explicit __shared_ptr(const __weak_ptr<_Yp>& __r)
			: _M_refcount(__r._M_refcount) 
		{
			_M_ptr = __r._M_ptr;
		}

		constexpr __shared_ptr(nullptr_t) noexcept : __shared_ptr() { }

		template<typename _Yp>
		__shared_ptr<_Tp> operator=(const __shared_ptr<_Yp>& __r) noexcept
		{
			_M_ptr = __r._M_ptr;
			_M_refcount = __r._M_refcount; // __shared_count::op= doesn't throw
			return *this;
		}

		__shared_ptr& operator=(__shared_ptr&& __r) noexcept
		{
			__shared_ptr(std::move(__r)).swap(*this);
			return *this;
		}

		template<class _Yp>
		__shared_ptr<_Tp> operator=(__shared_ptr<_Yp>&& __r) noexcept
		{
			__shared_ptr(std::move(__r)).swap(*this);
			return *this;
		}

		void reset() noexcept
		{
			__shared_ptr().swap(*this);
		}

		template<typename _Yp>
		__shared_ptr<_Yp> reset(_Yp* __p) // _Yp must be complete.
		{
			__glibcxx_assert(__p == 0 || __p != _M_ptr);
			__shared_ptr(__p).swap(*this);
		}

		element_type* get() const noexcept
		{
			return _M_ptr;
		}

		explicit operator bool() const // never throws
		{
			return _M_ptr == 0 ? false : true;
		}

		long use_count() const noexcept
		{
			return _M_refcount._M_get_use_count();
		}

		void swap(__shared_ptr<_Tp>& __other) noexcept
		{
			std::swap(_M_ptr, __other._M_ptr);
			_M_refcount._M_swap(__other._M_refcount);
		}

		/*template<typename _Tp1>
		bool owner_before(__shared_ptr<_Tp1> const& __rhs) const noexcept
		{
			return _M_refcount._M_less(__rhs._M_refcount);
		}

		template<typename _Tp1>
		bool owner_before(__weak_ptr<_Tp1> const& __rhs) const noexcept
		{
			return _M_refcount._M_less(__rhs._M_refcount);
		}*/

	protected:
		
		__shared_ptr(const __weak_ptr<_Tp>& __r, std::nothrow_t)
			: _M_refcount(__r._M_refcount, std::nothrow)
		{
			_M_ptr = _M_refcount._M_get_use_count() ? __r._M_ptr : nullptr;
		}

		friend class __weak_ptr<_Tp>;

	private:

		/*template<typename _Yp, typename _Yp2 = typename remove_cv<_Yp>::type>
		typename enable_if<__has_esft_base<_Yp2>::value>::type
			_M_enable_shared_from_this_with(_Yp* __p) noexcept
		{
			if (auto __base = __enable_shared_from_this_base(_M_refcount, __p))
				__base->_M_weak_assign(const_cast<_Yp2*>(__p), _M_refcount);
		}

		template<typename _Yp, typename _Yp2 = typename remove_cv<_Yp>::type>
		typename enable_if<!__has_esft_base<_Yp2>::value>::type
			_M_enable_shared_from_this_with(_Yp*) noexcept
		{ }*/

		template<typename _Tp1> friend class __shared_ptr;
		template<typename _Tp1> friend class __weak_ptr;

		element_type*				_M_ptr;        
		__shared_count	_M_refcount;

	};

	template<typename _Tp1, typename _Tp2>
	inline bool operator==(const __shared_ptr<_Tp1>& __a, const __shared_ptr<_Tp2>& __b) noexcept
	{
		return __a.get() == __b.get();
	}

	template<typename _Tp>
	inline bool operator==(const __shared_ptr<_Tp>& __a, nullptr_t) noexcept
	{
		return !__a;
	}

	template<typename _Tp>
	inline bool operator==(nullptr_t, const __shared_ptr<_Tp>& __a) noexcept
	{
		return !__a;
	}

	template<typename _Tp1, typename _Tp2>
	inline bool operator!=(const __shared_ptr<_Tp1>& __a, const __shared_ptr<_Tp2>& __b) noexcept
	{
		return __a.get() != __b.get();
	}

	template<typename _Tp>
	inline bool operator!=(const __shared_ptr<_Tp>& __a, nullptr_t) noexcept
	{
		return (bool)__a;
	}

	template<typename _Tp>
	inline bool operator!=(nullptr_t, const __shared_ptr<_Tp>& __a) noexcept
	{
		return (bool)__a;
	}

	template<typename _Tp, typename _Up>
	inline bool operator<(const __shared_ptr<_Tp>& __a, const __shared_ptr<_Up>& __b) noexcept
	{
		using _Tp_elt = typename __shared_ptr<_Tp>::element_type;
		using _Up_elt = typename __shared_ptr<_Up>::element_type;
		using _Vp = typename common_type<_Tp_elt*, _Up_elt*>::type;
		return std::less<_Vp>()(__a.get(), __b.get());
	}

	template<typename _Tp>
	inline bool operator<(const __shared_ptr<_Tp>& __a, nullptr_t) noexcept
	{
		using _Tp_elt = typename __shared_ptr<_Tp>::element_type;
		return std::less<_Tp_elt*>()(__a.get(), nullptr);
	}

	template<typename _Tp>
	inline bool operator<(nullptr_t, const __shared_ptr<_Tp>& __a) noexcept
	{
		using _Tp_elt = typename __shared_ptr<_Tp>::element_type;
		return std::less<_Tp_elt*>()(nullptr, __a.get());
	}

	template<typename _Tp1, typename _Tp2>
	inline bool operator<=(const __shared_ptr<_Tp1>& __a, const __shared_ptr<_Tp2>& __b) noexcept
	{
		return !(__b < __a);
	}

	template<typename _Tp>
	inline bool operator<=(const __shared_ptr<_Tp>& __a, nullptr_t) noexcept
	{
		return !(nullptr < __a);
	}

	template<typename _Tp>
	inline bool operator<=(nullptr_t, const __shared_ptr<_Tp>& __a) noexcept
	{
		return !(__a < nullptr);
	}

	template<typename _Tp1, typename _Tp2>
	inline bool operator>(const __shared_ptr<_Tp1>& __a, const __shared_ptr<_Tp2>& __b) noexcept
	{
		return (__b < __a);
	}

	template<typename _Tp>
	inline bool operator>(const __shared_ptr<_Tp>& __a, nullptr_t) noexcept
	{
		return nullptr < __a;
	}

	template<typename _Tp>
	inline bool operator>(nullptr_t, const __shared_ptr<_Tp>& __a) noexcept
	{
		return __a < nullptr;
	}

	template<typename _Tp1, typename _Tp2>
	inline bool operator>=(const __shared_ptr<_Tp1>& __a, const __shared_ptr<_Tp2>& __b) noexcept
	{
		return !(__a < __b);
	}

	template<typename _Tp>
	inline bool operator>=(const __shared_ptr<_Tp>& __a, nullptr_t) noexcept
	{
		return !(__a < nullptr);
	}

	template<typename _Tp>
	inline bool operator>=(nullptr_t, const __shared_ptr<_Tp>& __a) noexcept
	{
		return !(nullptr < __a);
	}

	template<typename _Tp>
	inline void swap(__shared_ptr<_Tp>& __a, __shared_ptr<_Tp>& __b) noexcept
	{
		__a.swap(__b);
	}

	template<typename _Tp, typename _Tp1>
	inline __shared_ptr<_Tp> static_pointer_cast(const __shared_ptr<_Tp1>& __r) noexcept
	{
		using _Sp = __shared_ptr<_Tp>;
		return _Sp(__r, static_cast<typename _Sp::element_type*>(__r.get()));
	}

	template<typename _Tp, typename _Tp1>
	inline __shared_ptr<_Tp> const_pointer_cast(const __shared_ptr<_Tp1>& __r) noexcept
	{
		using _Sp = __shared_ptr<_Tp>;
		return _Sp(__r, const_cast<typename _Sp::element_type*>(__r.get()));
	}

	template<typename _Tp, typename _Tp1>
	inline __shared_ptr<_Tp> dynamic_pointer_cast(const __shared_ptr<_Tp1>& __r) noexcept
	{
		using _Sp = __shared_ptr<_Tp>;
		if (auto* __p = dynamic_cast<typename _Sp::element_type*>(__r.get()))
			return _Sp(__r, __p);
		return _Sp();
	}

	template<typename _Tp>
	class __weak_ptr
	{
	public:
		using element_type = typename _Tp;

		constexpr __weak_ptr() noexcept
			: _M_ptr(nullptr), _M_refcount()
		{ }

		__weak_ptr(const __weak_ptr&) noexcept = default;

		~__weak_ptr() = default;

		template<typename _Yp>
		__weak_ptr(const __weak_ptr<_Yp>& __r) noexcept
			: _M_refcount(__r._M_refcount)
		{
			_M_ptr = __r.lock().get();
		}

		template<typename _Yp>
		__weak_ptr(const __shared_ptr<_Yp>& __r) noexcept
			: _M_ptr(__r._M_ptr), _M_refcount(__r._M_refcount)
		{ }

		__weak_ptr(__weak_ptr&& __r) noexcept
			: _M_ptr(__r._M_ptr), _M_refcount(std::move(__r._M_refcount))
		{
			__r._M_ptr = nullptr;
		}

		template<typename _Yp>
		__weak_ptr(__weak_ptr<_Yp>&& __r) noexcept
			: _M_ptr(__r.lock().get()), _M_refcount(std::move(__r._M_refcount))
		{
			__r._M_ptr = nullptr;
		}

		__weak_ptr& operator=(const __weak_ptr& __r) noexcept = default;

		template<typename _Yp>
		__weak_ptr<_Yp> operator=(const __weak_ptr<_Yp>& __r) noexcept
		{
			_M_ptr = __r.lock().get();
			_M_refcount = __r._M_refcount;
			return *this;
		}

		template<typename _Yp>
		__weak_ptr<_Yp> operator=(const __shared_ptr<_Yp>& __r) noexcept
		{
			_M_ptr = __r._M_ptr;
			_M_refcount = __r._M_refcount;
			return *this;
		}

		__weak_ptr& operator=(__weak_ptr&& __r) noexcept
		{
			_M_ptr = __r._M_ptr;
			_M_refcount = std::move(__r._M_refcount);
			__r._M_ptr = nullptr;
			return *this;
		}

		template<typename _Yp>
		__weak_ptr<_Yp> operator=(__weak_ptr<_Yp>&& __r) noexcept
		{
			_M_ptr = __r.lock().get();
			_M_refcount = std::move(__r._M_refcount);
			__r._M_ptr = nullptr;
			return *this;
		}

		__shared_ptr<_Tp> lock() const noexcept
		{
			return __shared_ptr<element_type>(*this, std::nothrow);
		}

		long use_count() const noexcept
		{
			return _M_refcount._M_get_use_count();
		}

		bool expired() const noexcept
		{
			return _M_refcount._M_get_use_count() == 0;
		}

		template<typename _Tp1>
		bool owner_before(const __shared_ptr<_Tp1>& __rhs) const noexcept
		{
			return _M_refcount._M_less(__rhs._M_refcount);
		}

		template<typename _Tp1>
		bool owner_before(const __weak_ptr<_Tp1>& __rhs) const noexcept
		{
			return _M_refcount._M_less(__rhs._M_refcount);
		}

		void reset() noexcept
		{
			__weak_ptr().swap(*this);
		}

		void swap(__weak_ptr& __s) noexcept
		{
			std::swap(_M_ptr, __s._M_ptr);
			_M_refcount._M_swap(__s._M_refcount);
		}

	private:
		
		void _M_assign(_Tp* __ptr, const __shared_count& __refcount) noexcept
		{
			if (use_count() == 0)
			{
				_M_ptr = __ptr;
				_M_refcount = __refcount;
			}
		}

		template<typename _Tp1> friend class __shared_ptr;
		template<typename _Tp1> friend class __weak_ptr;

		/*friend class __enable_shared_from_this<_Tp>;
		friend class enable_shared_from_this<_Tp>;*/

		element_type*			_M_ptr;         // Contained pointer.
		__weak_count	_M_refcount;    // Reference counter.
	};

	template<typename _Tp>
	inline void swap(__weak_ptr<_Tp>& __a, __weak_ptr<_Tp>& __b) noexcept
	{
		__a.swap(__b);
	}

	template<typename _Tp>
	class __enable_shared_from_this
	{
	protected:
		constexpr __enable_shared_from_this() noexcept
		{ }

		__enable_shared_from_this(const __enable_shared_from_this&) noexcept 
		{ }

		__enable_shared_from_this& operator=(const __enable_shared_from_this&) noexcept
		{
			return *this;
		}

		~__enable_shared_from_this()
		{ }

	public:
		__shared_ptr<_Tp> shared_from_this()
		{
			return __shared_ptr<_Tp>(this->_M_weak_this);
		}

		__shared_ptr<const _Tp> shared_from_this() const
		{
			return __shared_ptr<const _Tp>(this->_M_weak_this);
		}

		__weak_ptr<_Tp> weak_from_this() noexcept
		{
			return this->_M_weak_this;
		}

		__weak_ptr<const _Tp> weak_from_this() const noexcept
		{
			return this->_M_weak_this;
		}

	private:
		template<typename _Tp1>
		void _M_weak_assign(_Tp1* __p, const __shared_count& __n) const noexcept
		{
			_M_weak_this._M_assign(__p, __n);
		}

		friend const __enable_shared_from_this*
			__enable_shared_from_this_base(const __shared_count&, const __enable_shared_from_this* __p)
		{
			return __p;
		}

		template<typename> friend class __shared_ptr;

		mutable __weak_ptr<_Tp>  _M_weak_this;
	};


}	// namespace LFSP

#endif