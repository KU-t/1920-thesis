

namespace LFSP {

	class __counted_base;

	class __shared_count;
	
	class __weak_count
	{
	public:
		constexpr __weak_count() noexcept;

		__weak_count(const __shared_count& __r) noexcept;

		__weak_count(const __weak_count& __r) noexcept;

		__weak_count(__weak_count&& __r) noexcept;

		~__weak_count() noexcept;

		__weak_count& operator=(const __shared_count& __r) noexcept;

		__weak_count& operator=(const __weak_count& __r) noexcept;

		__weak_count& operator=(__weak_count&& __r) noexcept;

		void _M_swap(__weak_count& __r) noexcept;

		long _M_get_use_count() const noexcept;

		/*bool _M_less(const __weak_count& __rhs) const noexcept
		{
			return std::less<__counted_base*>()(this->_M_pi, __rhs._M_pi);
		}

		bool _M_less(const __shared_count& __rhs) const noexcept
		{
			return std::less<__counted_base*>()(this->_M_pi, __rhs._M_pi);
		}*/

		friend inline bool operator==(const __weak_count& __a, const __weak_count& __b) noexcept;

	private:
		friend class __shared_count;

		__counted_base*  _M_pi;
	};

}