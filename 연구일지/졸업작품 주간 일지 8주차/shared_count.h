
namespace LFSP {

	class __counted_base;

	class __weak_count;

	class __shared_count
	{
	public:
		constexpr __shared_count() noexcept;

		template<typename _Ptr>
		explicit __shared_count(_Ptr __p);

		inline explicit __shared_count(const __weak_count& __r);

		/*inline explicit __shared_count(const __weak_count& __r, std::nothrow_t)
			: _M_pi(__r._M_pi)
		{
			if (_M_pi != nullptr)
			{
				if (!_M_pi->_M_add_ref_lock_nothrow())
					_M_pi = nullptr;
			}
		}*/

		~__shared_count() noexcept;

		__shared_count(const __shared_count& __r) noexcept;

		__shared_count& operator=(const __shared_count& __r) noexcept;

		void _M_swap(__shared_count& __r) noexcept;

		long _M_get_use_count() const noexcept;

		/*bool _M_less(const __shared_count& __rhs) const noexcept
		{
			return std::less<__counted_base*>()(this->_M_pi, __rhs._M_pi);
		}

		bool _M_less(const __weak_count& __rhs) const noexcept
		{
			return std::less<__counted_base*>()(this->_M_pi, __rhs._M_pi);
		}*/

		friend inline bool operator==(const __shared_count& __a, const __shared_count& __b) noexcept;
		
	private:
		friend class __weak_count;

		__counted_base*  _M_pi;
	};

}