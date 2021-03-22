#pragma once
#include <atomic>

namespace LFSP {

	inline void bad_pointer() {
		std::cout << "bad_pointer";
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



	
	//class __shared_count;



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

}