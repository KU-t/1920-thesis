
#ifndef SHARED_PTR_H
#define SHARED_PTR_H 1

#include "shared_ptr_base.h"
//#include <atomic>

namespace LFSP {
	
	template<typename _Ch, typename _Tr, typename _Tp>
	inline std::basic_ostream<_Ch, _Tr>& operator<<(std::basic_ostream<_Ch, _Tr>& __os, const __shared_ptr<_Tp>& __p)
	{
		__os << __p.get();
		return __os;
	}

	template<typename _Tp>
	class shared_ptr : public __shared_ptr<_Tp>
	{
	public:
		using element_type = typename _Tp;

		constexpr shared_ptr() noexcept : __shared_ptr<_Tp>() { }

		shared_ptr(const shared_ptr&) noexcept = default;

		template<typename _Yp>
		explicit shared_ptr(_Yp* __p)
			: __shared_ptr<_Tp>(__p)
		{ }

		template<typename _Yp>
		shared_ptr(const shared_ptr<_Yp>& __r, element_type* __p) noexcept
			: __shared_ptr<_Tp>(__r, __p)
		{ }

		template<typename _Yp>
		shared_ptr(shared_ptr<_Yp>&& __r, element_type* __p) noexcept
			: __shared_ptr<_Tp>(std::move(__r), __p)
		{ }

		template<typename _Yp>
		shared_ptr(const shared_ptr<_Yp>& __r) noexcept
			: __shared_ptr<_Tp>(__r)
		{ }

		shared_ptr(shared_ptr&& __r) noexcept 
			: __shared_ptr<_Tp>(std::move(__r))
		{ }

		template<typename _Yp>
		shared_ptr(shared_ptr<_Yp>&& __r) noexcept 
			: __shared_ptr<_Tp>(std::move(__r))
		{ }

		template<typename _Yp>
		explicit shared_ptr(const weak_ptr<_Yp>& __r)
			: __shared_ptr<_Tp>(__r)
		{ }

		constexpr shared_ptr(nullptr_t) noexcept 
			: shared_ptr() 
		{ }

		shared_ptr& operator=(const shared_ptr&) noexcept = default;

		template<typename _Yp>
		shared_ptr<_Tp> operator=(const shared_ptr<_Yp>& __r) noexcept
		{
			this->__shared_ptr<_Tp>::operator=(__r);
			return *this;
		}

		shared_ptr& operator=(shared_ptr&& __r) noexcept
		{
			this->shared_ptr<_Tp>::operator=(std::move(__r));
			return *this;
		}

		template<class _Yp>
		shared_ptr<_Yp> operator=(shared_ptr<_Yp>&& __r) noexcept
		{
			this->shared_ptr<_Tp>::operator=(std::move(__r));
			return *this;
		}

	private:
		/*shared_ptr(const weak_ptr<_Tp>& __r, std::nothrow_t)
			: __shared_ptr<_Tp>(__r, std::nothrow)
		{ }*/

		friend class weak_ptr<_Tp>;

	};


	template<typename _Tp, typename _Up>
	inline bool operator==(const shared_ptr<_Tp>& __a, const shared_ptr<_Up>& __b) noexcept
	{
		return __a.get() == __b.get();
	}

	/// shared_ptr comparison with nullptr
	template<typename _Tp>
	inline bool operator==(const shared_ptr<_Tp>& __a, nullptr_t) noexcept
	{
		return !__a;
	}

	/// shared_ptr comparison with nullptr
	template<typename _Tp>
	inline bool operator==(nullptr_t, const shared_ptr<_Tp>& __a) noexcept
	{
		return !__a;
	}

	/// Inequality operator for shared_ptr objects, compares the stored pointers
	template<typename _Tp, typename _Up>
	inline bool operator!=(const shared_ptr<_Tp>& __a, const shared_ptr<_Up>& __b) noexcept
	{
		return __a.get() != __b.get();
	}

	/// shared_ptr comparison with nullptr
	template<typename _Tp>
	inline bool operator!=(const shared_ptr<_Tp>& __a, nullptr_t) noexcept
	{
		return (bool)__a;
	}

	/// shared_ptr comparison with nullptr
	template<typename _Tp>
	inline bool operator!=(nullptr_t, const shared_ptr<_Tp>& __a) noexcept
	{
		return (bool)__a;
	}

	/// Relational operator for shared_ptr objects, compares the stored pointers
	template<typename _Tp, typename _Up>
	inline bool operator<(const shared_ptr<_Tp>& __a, const shared_ptr<_Up>& __b) noexcept
	{
		using _Tp_elt = typename shared_ptr<_Tp>::element_type;
		using _Up_elt = typename shared_ptr<_Up>::element_type;
		using _Vp = typename std::common_type<_Tp_elt*, _Up_elt*>::type;
		return std::less<_Vp>()(__a.get(), __b.get());
	}

	/// shared_ptr comparison with nullptr
	template<typename _Tp>
	inline bool operator<(const shared_ptr<_Tp>& __a, nullptr_t) noexcept
	{
		using _Tp_elt = typename shared_ptr<_Tp>::element_type;
		return std::less<_Tp_elt*>()(__a.get(), nullptr);
	}

	/// shared_ptr comparison with nullptr
	template<typename _Tp>
	inline bool operator<(nullptr_t, const shared_ptr<_Tp>& __a) noexcept
	{
		using _Tp_elt = typename shared_ptr<_Tp>::element_type;
		return std::less<_Tp_elt*>()(nullptr, __a.get());
	}

	/// Relational operator for shared_ptr objects, compares the stored pointers
	template<typename _Tp, typename _Up>
	inline bool operator<=(const shared_ptr<_Tp>& __a, const shared_ptr<_Up>& __b) noexcept
	{
		return !(__b < __a);
	}

	/// shared_ptr comparison with nullptr
	template<typename _Tp>
	inline bool operator<=(const shared_ptr<_Tp>& __a, nullptr_t) noexcept
	{
		return !(nullptr < __a);
	}

	/// shared_ptr comparison with nullptr
	template<typename _Tp>
	inline bool operator<=(nullptr_t, const shared_ptr<_Tp>& __a) noexcept
	{
		return !(__a < nullptr);
	}

	/// Relational operator for shared_ptr objects, compares the stored pointers
	template<typename _Tp, typename _Up>
	inline bool operator>(const shared_ptr<_Tp>& __a, const shared_ptr<_Up>& __b) noexcept
	{
		return (__b < __a);
	}

	/// shared_ptr comparison with nullptr
	template<typename _Tp>
	inline bool operator>(const shared_ptr<_Tp>& __a, nullptr_t) noexcept
	{
		return nullptr < __a;
	}

	/// shared_ptr comparison with nullptr
	template<typename _Tp>
	inline bool operator>(nullptr_t, const shared_ptr<_Tp>& __a) noexcept
	{
		return __a < nullptr;
	}

	/// Relational operator for shared_ptr objects, compares the stored pointers
	template<typename _Tp, typename _Up>
	inline bool operator>=(const shared_ptr<_Tp>& __a, const shared_ptr<_Up>& __b) noexcept
	{
		return !(__a < __b);
	}

	/// shared_ptr comparison with nullptr
	template<typename _Tp>
	inline bool operator>=(const shared_ptr<_Tp>& __a, nullptr_t) noexcept
	{
		return !(__a < nullptr);
	}

	/// shared_ptr comparison with nullptr
	template<typename _Tp>
	inline bool operator>=(nullptr_t, const shared_ptr<_Tp>& __a) noexcept
	{
		return !(nullptr < __a);
	}

	// 20.7.2.2.8 shared_ptr specialized algorithms.

	/// Swap overload for shared_ptr
	template<typename _Tp>
	inline void swap(shared_ptr<_Tp>& __a, shared_ptr<_Tp>& __b) noexcept
	{
		__a.swap(__b);
	}

	template<typename _Tp, typename _Up>
	inline shared_ptr<_Tp> static_pointer_cast(const shared_ptr<_Up>& __r) noexcept
	{
		using _Sp = shared_ptr<_Tp>;
		return _Sp(__r, static_cast<typename _Sp::element_type*>(__r.get()));
	}

	template<typename _Tp, typename _Up>
	inline shared_ptr<_Tp> const_pointer_cast(const shared_ptr<_Up>& __r) noexcept
	{
		using _Sp = shared_ptr<_Tp>;
		return _Sp(__r, const_cast<typename _Sp::element_type*>(__r.get()));
	}

	template<typename _Tp, typename _Up>
	inline shared_ptr<_Tp> dynamic_pointer_cast(const shared_ptr<_Up>& __r) noexcept
	{
		using _Sp = shared_ptr<_Tp>;
		if (auto* __p = dynamic_cast<typename _Sp::element_type*>(__r.get()))
			return _Sp(__r, __p);
		return _Sp();
	}

	template<typename _Tp, typename _Up>
	inline shared_ptr<_Tp> reinterpret_pointer_cast(const shared_ptr<_Up>& __r) noexcept
	{
		using _Sp = shared_ptr<_Tp>;
		return _Sp(__r, reinterpret_cast<typename _Sp::element_type*>(__r.get()));
	}

	template<typename _Tp, typename _Up>
	inline shared_ptr<_Tp> static_pointer_cast(shared_ptr<_Up>&& __r) noexcept
	{
		using _Sp = shared_ptr<_Tp>;
		return _Sp(std::move(__r), static_cast<typename _Sp::element_type*>(__r.get()));
	}

	template<typename _Tp, typename _Up>
	inline shared_ptr<_Tp> const_pointer_cast(shared_ptr<_Up>&& __r) noexcept
	{
		using _Sp = shared_ptr<_Tp>;
		return _Sp(std::move(__r), const_cast<typename _Sp::element_type*>(__r.get()));
	}

	template<typename _Tp, typename _Up>
	inline shared_ptr<_Tp> dynamic_pointer_cast(shared_ptr<_Up>&& __r) noexcept
	{
		using _Sp = shared_ptr<_Tp>;
		if (auto* __p = dynamic_cast<typename _Sp::element_type*>(__r.get()))
			return _Sp(std::move(__r), __p);
		return _Sp();
	}

	template<typename _Tp, typename _Up>
	inline shared_ptr<_Tp> reinterpret_pointer_cast(shared_ptr<_Up>&& __r) noexcept
	{
		using _Sp = shared_ptr<_Tp>;
		return _Sp(std::move(__r), reinterpret_cast<typename _Sp::element_type*>(__r.get()));
	}



	template<typename _Tp>
	class weak_ptr : public __weak_ptr<_Tp>
	{
	public:
		constexpr weak_ptr() noexcept = default;

		template<typename _Yp>
		weak_ptr(const shared_ptr<_Yp>& __r) noexcept
			: __weak_ptr<_Tp>(__r)
		{ }

		weak_ptr(const weak_ptr&) noexcept = default;

		template<typename _Yp>
		weak_ptr(const weak_ptr<_Yp>& __r) noexcept
			: __weak_ptr<_Tp>(__r)
		{ }

		weak_ptr(weak_ptr&&) noexcept = default;

		template<typename _Yp>
		weak_ptr(weak_ptr<_Yp>&& __r) noexcept
			: __weak_ptr<_Tp>(std::move(__r))
		{ }

		weak_ptr& operator=(const weak_ptr& __r) noexcept = default;

		template<typename _Yp>
		weak_ptr<_Yp>& operator=(const weak_ptr<_Yp>& __r) noexcept
		{
			this->__weak_ptr<_Tp>::operator=(__r);
			return *this;
		}

		template<typename _Yp>
		const shared_ptr<_Yp>& operator=(const shared_ptr<_Yp>& __r) noexcept
		{
			this->__weak_ptr<_Tp>::operator=(__r);
			return *this;
		}

		//shared_ptr& operator=(shared_ptr&& __r) noexcept 
		//{
		//	this->shared_ptr<_Tp>::operator=(std::move(__r));
		//	return *this;
		//}

		template<typename _Yp>
		shared_ptr<_Yp> operator=(shared_ptr<_Yp>&& __r) noexcept
		{
			this->__weak_ptr<_Tp>::operator=(std::move(__r));
			return *this;
		}

		shared_ptr<_Tp> lock() const noexcept
		{
			return shared_ptr<_Tp>(*this, std::nothrow);
		}
	};


	template<typename _Tp>
	class enable_shared_from_this
	{
	protected:
		constexpr enable_shared_from_this() noexcept { }

		enable_shared_from_this(const enable_shared_from_this&) noexcept { }

		enable_shared_from_this& operator=(const enable_shared_from_this&) noexcept
		{
			return *this;
		}

		~enable_shared_from_this() { }

	public:
		shared_ptr<_Tp> shared_from_this()
		{
			return shared_ptr<_Tp>(this->_M_weak_this);
		}

		shared_ptr<const _Tp> shared_from_this() const
		{
			return shared_ptr<const _Tp>(this->_M_weak_this);
		}


		weak_ptr<_Tp> weak_from_this() noexcept
		{
			return this->_M_weak_this;
		}

		weak_ptr<const _Tp> weak_from_this() const noexcept
		{
			return this->_M_weak_this;
		}


	private:
		template<typename _Tp1>
		void _M_weak_assign(_Tp1* __p, const __shared_count& __n) const noexcept
		{
			_M_weak_this._M_assign(__p, __n);
		}

		friend const enable_shared_from_this*
			__enable_shared_from_this_base(const __shared_count&, const enable_shared_from_this* __p)
		{
			return __p;
		}

		template<typename>
		friend class __shared_ptr;

		mutable weak_ptr<_Tp>  _M_weak_this;
	};

}	// namespace

#endif