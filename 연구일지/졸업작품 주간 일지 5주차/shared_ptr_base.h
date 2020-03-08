
#ifndef _SHARED_PTR_BASE_H
#define _SHARED_PTR_BASE_H 1

#include <typeinfo>
#include <atomic>

inline void bad_weak_ptr() {
	printf("bad_weak_ptr()");
	system("puase");
}


namespace std {

	class _Sp_counted_base {
	public:
		_Sp_counted_base() noexcept 
			: _M_use_count(1), _M_weak_count(1)
		{ }

		virtual	~_Sp_counted_base() noexcept 
		{ }

		// Called when _M_use_count drops to zero, to release the resources
		// managed by *this.
		virtual void _M_dispose() noexcept = 0;

		// Called when _M_weak_count drops to zero.
		virtual void _M_destroy() noexcept
		{
			delete this;
		}

		virtual void* _M_get_deleter(const std::type_info&) noexcept = 0;

		void _M_add_ref_copy()
		{
			_M_use_count++;
		}

		void _M_add_ref_lock();

		bool _M_add_ref_lock_nothrow();

		void _M_release() noexcept
		{
			// Be race-detector-friendly.  For more info see bits/c++config.
			if ( 1 == _M_use_count--)
			{
				_M_dispose();
				atomic_thread_fence(memory_order_acquire);

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

		void _M_weak_release() noexcept							/**********************원래코드랑 비교********************/
		{
			if (1 == _M_weak_count--) {
				atomic_thread_fence(memory_order_acq_rel);
				_M_destroy();
			}
		}

		long _M_get_use_count() const noexcept
		{
			// No memory barrier is used here so there is no synchronization
			// with other threads.
			return _M_use_count.load(memory_order_relaxed);
		}

	private:
		_Sp_counted_base(_Sp_counted_base const&) = delete;
		_Sp_counted_base& operator=(_Sp_counted_base const&) = delete;

		atomic_int  _M_use_count;     // #shared
		atomic_int  _M_weak_count;    // #weak + (#shared != 0)
	};

inline void _Sp_counted_base::_M_add_ref_lock()
	{
		// Perform lock-free add-if-not-zero operation.
		int __count = _M_get_use_count();
		do {
			if (__count == 0)	bad_weak_ptr();
			// Replace the current counter value with the old value + 1, as
			// long as it's not changed meanwhile.
		} while (!atomic_compare_exchange_strong(&_M_use_count, &__count, __count + 1));
	}

	inline bool _Sp_counted_base::_M_add_ref_lock_nothrow()
	{
		// Perform lock-free add-if-not-zero operation.
		int __count = _M_get_use_count();
		do {
			if (__count == 0)
				return false;
			// Replace the current counter value with the old value + 1, as
			// long as it's not changed meanwhile.
		} while (!atomic_compare_exchange_strong(&_M_use_count, &__count, __count + 1));
		return true;
	}

	// Forward declarations.
	template<typename _Tp>
	class __shared_ptr;

	template<typename _Tp>
	class __weak_ptr;

	template<typename _Tp>
	class __enable_shared_from_this;

	template<typename _Tp>
	class shared_ptr;

	template<typename _Tp>
	class weak_ptr;

	template<typename _Tp>
	struct owner_less;

	template<typename _Tp>
	class enable_shared_from_this;

	class __weak_count;

	class __shared_count;


	// Counted ptr with no deleter or allocator support
	template<typename _Ptr>
	class _Sp_counted_ptr final : public _Sp_counted_base
	{
	public:
		explicit _Sp_counted_ptr(_Ptr __p) noexcept : _M_ptr(__p) { }

		virtual void _M_dispose() noexcept
		{
			delete _M_ptr;
		}

		virtual void _M_destroy() noexcept
		{
			delete this;
		}

		virtual void* _M_get_deleter(const std::type_info&) noexcept
		{
			return nullptr;
		}

		_Sp_counted_ptr(const _Sp_counted_ptr&) = delete;
		_Sp_counted_ptr& operator=(const _Sp_counted_ptr&) = delete;

	private:
		_Ptr             _M_ptr;
	};

	
	template<>
	inline void _Sp_counted_ptr<nullptr_t>::_M_dispose() noexcept { }

	class __shared_count
	{

	public:
		constexpr __shared_count() noexcept : _M_pi(0)
		{ }

		template<typename _Ptr>
		explicit __shared_count(_Ptr __p) : _M_pi(0)
		{
			try {
				_M_pi = new _Sp_counted_ptr<_Ptr>(__p);
			}
			catch(...) {
				delete __p;
				bad_weak_ptr();
			}
		}

		//template<typename _Ptr>
		//__shared_count(_Ptr __p, /* is_array = */ false_type) 
		//	: __shared_count(__p) 
		//{ }

		//template<typename _Ptr>
		//__shared_count(_Ptr __p, /* is_array = */ true_type) 
		//	: __shared_count(__p, __sp_array_delete{}, allocator<void>()) 
		//{ }
		//               
		////
		//template<typename _Ptr, typename = typename __not_alloc_shared_tag<_Deleter>::type>
		//__shared_count(_Ptr __p) : __shared_count(__p, std::move(), allocator<void>()) { }

		////
		//template<typename _Ptr, typename _Alloc, typename = typename __not_alloc_shared_tag<_Deleter>::type>
		//__shared_count(_Ptr __p, _Deleter __d, _Alloc __a) : _M_pi(0)
		//{
		//	typedef _Sp_counted_deleter<_Ptr, _Deleter, _Alloc, _Lp> _Sp_cd_type;
		//	__try
		//	{
		//		typename _Sp_cd_type::__allocator_type __a2(__a);
		//		auto __guard = std::__allocate_guarded(__a2);
		//		_Sp_cd_type* __mem = __guard.get();
		//		::new (__mem) _Sp_cd_type(__p, std::move(__d), std::move(__a));
		//		_M_pi = __mem;
		//		__guard = nullptr;
		//	}
		//	__catch(...)
		//	{
		//		__d(__p); // Call _Deleter on __p.
		//		__throw_exception_again;
		//	}
		//}

		////
		//template<typename _Tp, typename _Alloc, typename... _Args>
		//__shared_count(_Tp*& __p, _Sp_alloc_shared_tag<_Alloc> __a, _Args&&... __args)
		//{
		//	typedef _Sp_counted_ptr_inplace<_Tp, _Alloc, _Lp> _Sp_cp_type;
		//	typename _Sp_cp_type::__allocator_type __a2(__a._M_a);
		//	auto __guard = std::__allocate_guarded(__a2);
		//	_Sp_cp_type* __mem = __guard.get();
		//	auto __pi = ::new (__mem)
		//		_Sp_cp_type(__a._M_a, std::forward<_Args>(__args)...);
		//	__guard = nullptr;
		//	_M_pi = __pi;
		//	__p = __pi->_M_ptr();
		//}


		// Throw bad_weak_ptr when __r._M_get_use_count() == 0.
		inline explicit __shared_count(const __weak_count& __r) 
			: _M_pi(__r._M_pi)
		{
			if (_M_pi != nullptr)
				_M_pi->_M_add_ref_lock();
			else
				bad_weak_ptr();
		}
		// Does not throw if __r._M_get_use_count() == 0, caller must check.
		inline explicit __shared_count(const __weak_count& __r, std::nothrow_t) 
			: _M_pi(__r._M_pi)
		{
			if (_M_pi != nullptr)
				if (!_M_pi->_M_add_ref_lock_nothrow())
					_M_pi = nullptr;
		}

		~__shared_count() noexcept
		{
			if (_M_pi != nullptr) _M_pi->_M_release();
		}

		__shared_count(const __shared_count& __r) noexcept
			: _M_pi(__r._M_pi)
		{
			if (_M_pi != 0)
				_M_pi->_M_add_ref_copy();
		}

		__shared_count& operator=(const __shared_count& __r) noexcept
		{
			_Sp_counted_base* __tmp = __r._M_pi;
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
			_Sp_counted_base* __tmp = __r._M_pi;
			__r._M_pi = _M_pi;
			_M_pi = __tmp;
		}

		long _M_get_use_count() const noexcept
		{
			return _M_pi != 0 ? _M_pi->_M_get_use_count() : 0;
		}

		bool _M_less(const __shared_count& __rhs) const noexcept
		{
			return std::less<_Sp_counted_base*>()(this->_M_pi, __rhs._M_pi);
		}

		bool _M_less(const __weak_count& __rhs) const noexcept
		{
			return std::less<_Sp_counted_base*>()(this->_M_pi, __rhs._M_pi);
		}

		// Friend function injected into enclosing namespace and found by ADL
		friend inline bool operator==(const __shared_count& __a, const __shared_count& __b) noexcept
		{
			return __a._M_pi == __b._M_pi;
		}

	private:
		friend class __weak_count;

		_Sp_counted_base*  _M_pi;
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
			_Sp_counted_base* __tmp = __r._M_pi;
			if (__tmp != nullptr)
				__tmp->_M_weak_add_ref();
			if (_M_pi != nullptr)
				_M_pi->_M_weak_release();
			_M_pi = __tmp;
			return *this;
		}

		__weak_count& operator=(const __weak_count& __r) noexcept
		{
			_Sp_counted_base* __tmp = __r._M_pi;
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
			_Sp_counted_base* __tmp = __r._M_pi;
			__r._M_pi = _M_pi;
			_M_pi = __tmp;
		}

		long _M_get_use_count() const noexcept
		{
			return _M_pi != nullptr ? _M_pi->_M_get_use_count() : 0;
		}

		bool _M_less(const __weak_count& __rhs) const noexcept
		{
			return std::less<_Sp_counted_base*>()(this->_M_pi, __rhs._M_pi);
		}

		bool _M_less(const __shared_count& __rhs) const noexcept
		{
			return std::less<_Sp_counted_base*>()(this->_M_pi, __rhs._M_pi);
		}

		// Friend function injected into enclosing namespace and found by ADL
		friend inline bool operator==(const __weak_count& __a, const __weak_count& __b) noexcept
		{
			return __a._M_pi == __b._M_pi;
		}

	private:
		friend class __shared_count;

		_Sp_counted_base*  _M_pi;
	};
	
#define __cpp_lib_shared_ptr_arrays 201611L

	// Helper traits for shared_ptr of array:

	// A pointer type Y* is said to be compatible with a pointer type T* when
	// either Y* is convertible to T* or Y is U[N] and T is U cv [].
	template<typename _Yp_ptr, typename _Tp_ptr>
	struct __sp_compatible_with
		: false_type
	{ };

	template<typename _Yp, typename _Tp>
	struct __sp_compatible_with<_Yp*, _Tp*>
		: is_convertible<_Yp*, _Tp*>::type
	{ };

	template<typename _Up, size_t _Nm>
	struct __sp_compatible_with<_Up(*)[_Nm], _Up(*)[]>
		: true_type
	{ };

	template<typename _Up, size_t _Nm>
	struct __sp_compatible_with<_Up(*)[_Nm], const _Up(*)[]>
		: true_type
	{ };

	template<typename _Up, size_t _Nm>
	struct __sp_compatible_with<_Up(*)[_Nm], volatile _Up(*)[]>
		: true_type
	{ };

	template<typename _Up, size_t _Nm>
	struct __sp_compatible_with<_Up(*)[_Nm], const volatile _Up(*)[]>
		: true_type
	{ };

	// Test conversion from Y(*)[N] to U(*)[N] without forming invalid type Y[N].
	template<typename _Up, size_t _Nm, typename _Yp, typename = void>
	struct __sp_is_constructible_arrN
		: false_type
	{ };

	template<typename _Up, size_t _Nm, typename _Yp>
	struct __sp_is_constructible_arrN<_Up, _Nm, _Yp, __void_t<_Yp[_Nm]>>
		: is_convertible<_Yp(*)[_Nm], _Up(*)[_Nm]>::type
	{ };

	// Test conversion from Y(*)[] to U(*)[] without forming invalid type Y[].
	template<typename _Up, typename _Yp, typename = void>
	struct __sp_is_constructible_arr
		: false_type
	{ };

	template<typename _Up, typename _Yp>
	struct __sp_is_constructible_arr<_Up, _Yp, __void_t<_Yp[]>>
		: is_convertible<_Yp(*)[], _Up(*)[]>::type
	{ };

	// Trait to check if shared_ptr<T> can be constructed from Y*.
	template<typename _Tp, typename _Yp>
	struct __sp_is_constructible;

	// When T is U[N], Y(*)[N] shall be convertible to T*;
	template<typename _Up, size_t _Nm, typename _Yp>
	struct __sp_is_constructible<_Up[_Nm], _Yp>
		: __sp_is_constructible_arrN<_Up, _Nm, _Yp>::type
	{ };

	// when T is U[], Y(*)[] shall be convertible to T*;
	template<typename _Up, typename _Yp>
	struct __sp_is_constructible<_Up[], _Yp>
		: __sp_is_constructible_arr<_Up, _Yp>::type
	{ };

	// otherwise, Y* shall be convertible to T*.
	template<typename _Tp, typename _Yp>
	struct __sp_is_constructible
		: is_convertible<_Yp*, _Tp*>::type
	{ };

	template<typename _Tp>
	class __shared_ptr
	{
	public:
		using element_type = typename remove_extent<_Tp>::type;

	private:
		// Constraint for taking ownership of a pointer of type _Yp*:
		template<typename _Yp>
		using _SafeConv = typename enable_if<__sp_is_constructible<_Tp, _Yp>::value>::type;

		// Constraint for construction from shared_ptr and weak_ptr:
		template<typename _Yp, typename _Res = void>
		using _Compatible = typename enable_if<__sp_compatible_with<_Yp*, _Tp*>::value, _Res>::type;

		// Constraint for assignment from shared_ptr and weak_ptr:
		template<typename _Yp>
		using _Assignable = _Compatible<_Yp, __shared_ptr&>;

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

#if __cplusplus > 201402L
		using weak_type = __weak_ptr<_Tp>;
#endif

		constexpr __shared_ptr() noexcept 
			: _M_ptr(0), _M_refcount()
		{ }

		template<typename _Yp, typename = _SafeConv<_Yp>>
		explicit __shared_ptr(_Yp* __p) 
			: _M_ptr(__p), _M_refcount(__p, typename is_array<_Tp>::type())
		{
			static_assert(!is_void<_Yp>::value, "incomplete type");
			static_assert(sizeof(_Yp) > 0, "incomplete type");
			_M_enable_shared_from_this_with(__p);
		}

		// Aliasing constructor
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

		template<typename _Yp, typename = _Compatible<_Yp>>
		__shared_ptr(const __shared_ptr<_Yp>& __r) noexcept
			: _M_ptr(__r._M_ptr), _M_refcount(__r._M_refcount)
		{ }

		__shared_ptr(__shared_ptr&& __r) noexcept 
			: _M_ptr(__r._M_ptr), _M_refcount()
		{
			_M_refcount._M_swap(__r._M_refcount);
			__r._M_ptr = 0;
		}

		template<typename _Yp, typename = _Compatible<_Yp>>
		__shared_ptr(__shared_ptr<_Yp>&& __r) noexcept
			: _M_ptr(__r._M_ptr), _M_refcount()
		{
			_M_refcount._M_swap(__r._M_refcount);
			__r._M_ptr = 0;
		}

		template<typename _Yp, typename = _Compatible<_Yp>>
		explicit __shared_ptr(const __weak_ptr<_Yp>& __r)
			: _M_refcount(__r._M_refcount) // may throw
		{
			// It is now safe to copy __r._M_ptr, as
			// _M_refcount(__r._M_refcount) did not throw.
			_M_ptr = __r._M_ptr;
		}
		
		constexpr __shared_ptr(nullptr_t) noexcept : __shared_ptr() { }

		template<typename _Yp>
		_Assignable<_Yp> operator=(const __shared_ptr<_Yp>& __r) noexcept
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
		_Assignable<_Yp> operator=(__shared_ptr<_Yp>&& __r) noexcept
		{
			__shared_ptr(std::move(__r)).swap(*this);
			return *this;
		}

		void reset() noexcept
		{
			__shared_ptr().swap(*this);
		}

		template<typename _Yp>
		_SafeConv<_Yp> reset(_Yp* __p) // _Yp must be complete.
		{
			// Catch self-reset errors.
			__glibcxx_assert(__p == 0 || __p != _M_ptr);
			__shared_ptr(__p).swap(*this);
		}

		/// Return the stored pointer.
		element_type* get() const noexcept
		{
			return _M_ptr;
		}

		/// Return true if the stored pointer is not null.
		explicit operator bool() const // never throws
		{
			return _M_ptr == 0 ? false : true;
		}

		/// If *this owns a pointer, return the number of owners, otherwise zero.
		long use_count() const noexcept
		{
			return _M_refcount._M_get_use_count();
		}

		/// Exchange both the owned pointer and the stored pointer.
		void swap(__shared_ptr<_Tp>& __other) noexcept
		{
			std::swap(_M_ptr, __other._M_ptr);
			_M_refcount._M_swap(__other._M_refcount);
		}

		/** @brief Define an ordering based on ownership.
		 *
		 * This function defines a strict weak ordering between two shared_ptr
		 * or weak_ptr objects, such that one object is less than the other
		 * unless they share ownership of the same pointer, or are both empty.
		 * @{
		*/
		template<typename _Tp1>
		bool owner_before(__shared_ptr<_Tp1> const& __rhs) const noexcept
		{
			return _M_refcount._M_less(__rhs._M_refcount);
		}

		template<typename _Tp1>
		bool owner_before(__weak_ptr<_Tp1> const& __rhs) const noexcept
		{
			return _M_refcount._M_less(__rhs._M_refcount);
		}
		// @}

	protected:
		
		// This constructor is used by __weak_ptr::lock() and
		// shared_ptr::shared_ptr(const weak_ptr&, std::nothrow_t).
		__shared_ptr(const __weak_ptr<_Tp>& __r, std::nothrow_t)
			: _M_refcount(__r._M_refcount, std::nothrow)
		{
			_M_ptr = _M_refcount._M_get_use_count() ? __r._M_ptr : nullptr;
		}

		friend class __weak_ptr<_Tp>;

	private:

		template<typename _Yp>
		using __esft_base_t 
			= decltype(__enable_shared_from_this_base(std::declval<const __shared_count&>(), std::declval<_Yp*>()));

		// Detect an accessible and unambiguous enable_shared_from_this base.
		template<typename _Yp, typename = void>
		struct __has_esft_base
			: false_type 
		{ };

		template<typename _Yp>
		struct __has_esft_base<_Yp, __void_t<__esft_base_t<_Yp>>>
			: __not_<is_array<_Tp>> 
		{ }; // No enable shared_from_this for arrays

		template<typename _Yp, typename _Yp2 = typename remove_cv<_Yp>::type>
		typename enable_if<__has_esft_base<_Yp2>::value>::type
		_M_enable_shared_from_this_with(_Yp* __p) noexcept
		{
			if (auto __base = __enable_shared_from_this_base(_M_refcount, __p))
				__base->_M_weak_assign(const_cast<_Yp2*>(__p), _M_refcount);
		}

		template<typename _Yp, typename _Yp2 = typename remove_cv<_Yp>::type>
		typename enable_if<!__has_esft_base<_Yp2>::value>::type
		_M_enable_shared_from_this_with(_Yp*) noexcept
		{ }

		void* _M_get_deleter(const std::type_info& __ti) const noexcept
		{
			return _M_refcount._M_get_deleter(__ti);
		}

		template<typename _Tp1> friend class __shared_ptr;
		template<typename _Tp1> friend class __weak_ptr;

		element_type*	   _M_ptr;         // Contained pointer.
		__shared_count  _M_refcount;    // Reference counter.
	};


	// 20.7.2.2.7 shared_ptr comparisons
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
		return less<_Vp>()(__a.get(), __b.get());
	}

	template<typename _Tp>
	inline bool operator<(const __shared_ptr<_Tp>& __a, nullptr_t) noexcept
	{
		using _Tp_elt = typename __shared_ptr<_Tp>::element_type;
		return less<_Tp_elt*>()(__a.get(), nullptr);
	}

	template<typename _Tp>
	inline bool operator<(nullptr_t, const __shared_ptr<_Tp>& __a) noexcept
	{
		using _Tp_elt = typename __shared_ptr<_Tp>::element_type;
		return less<_Tp_elt*>()(nullptr, __a.get());
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

	// 20.7.2.2.8 shared_ptr specialized algorithms.
	template<typename _Tp>
	inline void swap(__shared_ptr<_Tp>& __a, __shared_ptr<_Tp>& __b) noexcept
	{
		__a.swap(__b);
	}

	// 20.7.2.2.9 shared_ptr casts

	// The seemingly equivalent code:
	// shared_ptr<_Tp>(static_cast<_Tp*>(__r.get()))
	// will eventually result in undefined behaviour, attempting to
	// delete the same object twice.
	/// static_pointer_cast
	template<typename _Tp, typename _Tp1>
	inline __shared_ptr<_Tp> static_pointer_cast(const __shared_ptr<_Tp1>& __r) noexcept
	{
		using _Sp = __shared_ptr<_Tp>;
		return _Sp(__r, static_cast<typename _Sp::element_type*>(__r.get()));
	}

	// The seemingly equivalent code:
	// shared_ptr<_Tp>(const_cast<_Tp*>(__r.get()))
	// will eventually result in undefined behaviour, attempting to
	// delete the same object twice.
	/// const_pointer_cast
	template<typename _Tp, typename _Tp1>
	inline __shared_ptr<_Tp> const_pointer_cast(const __shared_ptr<_Tp1>& __r) noexcept
	{
		using _Sp = __shared_ptr<_Tp>;
		return _Sp(__r, const_cast<typename _Sp::element_type*>(__r.get()));
	}

	// The seemingly equivalent code:
	// shared_ptr<_Tp>(dynamic_cast<_Tp*>(__r.get()))
	// will eventually result in undefined behaviour, attempting to
	// delete the same object twice.
	/// dynamic_pointer_cast
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
		template<typename _Yp, typename _Res = void>
		using _Compatible = typename enable_if<__sp_compatible_with<_Yp*, _Tp*>::value, _Res>::type;

		// Constraint for assignment from shared_ptr and weak_ptr:
		template<typename _Yp>
		using _Assignable = _Compatible<_Yp, __weak_ptr&>;

	public:
		using element_type = typename remove_extent<_Tp>::type;

		constexpr __weak_ptr() noexcept
			: _M_ptr(nullptr), _M_refcount()
		{ }

		__weak_ptr(const __weak_ptr&) noexcept = default;

		~__weak_ptr() = default;

		// The "obvious" converting constructor implementation:
		//
		//  template<typename _Tp1>
		//    __weak_ptr(const __weak_ptr<_Tp1>& __r)
		//    : _M_ptr(__r._M_ptr), _M_refcount(__r._M_refcount) // never throws
		//    { }
		//
		// has a serious problem.
		//
		//  __r._M_ptr may already have been invalidated. The _M_ptr(__r._M_ptr)
		//  conversion may require access to *__r._M_ptr (virtual inheritance).
		//
		// It is not possible to avoid spurious access violations since
		// in multithreaded programs __r._M_ptr may be invalidated at any point.
		template<typename _Yp, typename = _Compatible<_Yp>>
		__weak_ptr(const __weak_ptr<_Yp>& __r) noexcept
			: _M_refcount(__r._M_refcount)
		{
			_M_ptr = __r.lock().get();
		}

		template<typename _Yp, typename = _Compatible<_Yp>>
		__weak_ptr(const __shared_ptr<_Yp>& __r) noexcept
			: _M_ptr(__r._M_ptr), _M_refcount(__r._M_refcount)
		{ }

		__weak_ptr(__weak_ptr&& __r) noexcept
			: _M_ptr(__r._M_ptr), _M_refcount(std::move(__r._M_refcount))
		{
			__r._M_ptr = nullptr;
		}

		template<typename _Yp, typename = _Compatible<_Yp>>
		__weak_ptr(__weak_ptr<_Yp>&& __r) noexcept
			: _M_ptr(__r.lock().get()), _M_refcount(std::move(__r._M_refcount))
		{
			__r._M_ptr = nullptr;
		}

		__weak_ptr& operator=(const __weak_ptr& __r) noexcept = default;

		template<typename _Yp>
		_Assignable<_Yp> operator=(const __weak_ptr<_Yp>& __r) noexcept
		{
			_M_ptr = __r.lock().get();
			_M_refcount = __r._M_refcount;
			return *this;
		}

		template<typename _Yp>
		_Assignable<_Yp> operator=(const __shared_ptr<_Yp>& __r) noexcept
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
		_Assignable<_Yp> operator=(__weak_ptr<_Yp>&& __r) noexcept
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
		// Used by __enable_shared_from_this.
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
		friend class __enable_shared_from_this<_Tp>;
		friend class enable_shared_from_this<_Tp>;

		element_type*	 _M_ptr;         // Contained pointer.
		__weak_count  _M_refcount;    // Reference counter.
	};

	// 20.7.2.3.6 weak_ptr specialized algorithms.
	template<typename _Tp>
	inline void swap(__weak_ptr<_Tp>& __a, __weak_ptr<_Tp>& __b) noexcept
	{
		__a.swap(__b);
	}

	template<typename _Tp, typename _Tp1>
	struct _Sp_owner_less : public binary_function<_Tp, _Tp, bool>
	{
		bool operator()(const _Tp& __lhs, const _Tp& __rhs) const noexcept
		{
			return __lhs.owner_before(__rhs);
		}

		bool operator()(const _Tp& __lhs, const _Tp1& __rhs) const noexcept
		{
			return __lhs.owner_before(__rhs);
		}

		bool operator()(const _Tp1& __lhs, const _Tp& __rhs) const noexcept
		{
			return __lhs.owner_before(__rhs);
		}
	};

	template<>
	struct _Sp_owner_less<void, void>
	{
		template<typename _Tp, typename _Up>
		auto operator()(const _Tp& __lhs, const _Up& __rhs) const noexcept
			-> decltype(__lhs.owner_before(__rhs))
		{
			return __lhs.owner_before(__rhs);
		}

		using is_transparent = void;
	};

	template<typename _Tp>
	struct owner_less<__shared_ptr<_Tp>>
		: public _Sp_owner_less<__shared_ptr<_Tp>, __weak_ptr<_Tp>>
	{ };

	template<typename _Tp>
	struct owner_less<__weak_ptr<_Tp>>
		: public _Sp_owner_less<__weak_ptr<_Tp>, __shared_ptr<_Tp>>
	{ };


	template<typename _Tp>
	class __enable_shared_from_this
	{
	protected:
		constexpr __enable_shared_from_this() noexcept { }

		__enable_shared_from_this(const __enable_shared_from_this&) noexcept { }

		__enable_shared_from_this& operator=(const __enable_shared_from_this&) noexcept
		{
			return *this;
		}

		~__enable_shared_from_this() { }

	public:
		__shared_ptr<_Tp> shared_from_this()
		{
			return __shared_ptr<_Tp>(this->_M_weak_this);
		}

		__shared_ptr<const _Tp> shared_from_this() const
		{
			return __shared_ptr<const _Tp>(this->_M_weak_this);
		}

#if __cplusplus > 201402L || !defined(__STRICT_ANSI__) // c++1z or gnu++11
		__weak_ptr<_Tp> weak_from_this() noexcept
		{
			return this->_M_weak_this;
		}

		__weak_ptr<const _Tp> weak_from_this() const noexcept
		{
			return this->_M_weak_this;
		}
#endif

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

		template<typename>
		friend class __shared_ptr;

		mutable __weak_ptr<_Tp>  _M_weak_this;
	};

	/// std::hash specialization for __shared_ptr.
	template<typename _Tp>
	struct hash<__shared_ptr<_Tp>>
	: public __hash_base<size_t, __shared_ptr<_Tp>>
	{
		size_t operator()(const __shared_ptr<_Tp>& __s) const noexcept
		{
			return hash<typename __shared_ptr<_Tp>::element_type*>()(
				__s.get());
		}
	};

} // namespace

#endif // _SHARED_PTR_BASE_H