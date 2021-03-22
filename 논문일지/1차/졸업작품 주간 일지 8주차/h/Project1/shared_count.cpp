#include "shared_count.h"
#include "weak_count.h"
#include "count_base.h"

constexpr LFSP::__shared_count::__shared_count() noexcept
	: _M_pi(0)
{ }

inline LFSP::__shared_count::__shared_count(const __weak_count & __r)
	: _M_pi(__r._M_pi)
{
	if (_M_pi != nullptr)
		_M_pi->_M_add_ref_lock();

	else
		bad_pointer();
}

LFSP::__shared_count::~__shared_count() noexcept
{
	if (_M_pi != nullptr)
		_M_pi->_M_release();
}

LFSP::__shared_count::__shared_count(const __shared_count & __r) noexcept
	: _M_pi(__r._M_pi)
{
	if (_M_pi != 0)
		_M_pi->_M_add_ref_copy();
}

LFSP::__shared_count & LFSP::__shared_count::operator=(const __shared_count & __r) noexcept
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

void LFSP::__shared_count::_M_swap(__shared_count & __r) noexcept
{
	__counted_base* __tmp = __r._M_pi;
	__r._M_pi = _M_pi;
	_M_pi = __tmp;
}

long LFSP::__shared_count::_M_get_use_count() const noexcept
{
	return _M_pi != 0 ? _M_pi->_M_get_use_count() : 0;
}

bool LFSP::operator==(const __shared_count & __a, const __shared_count & __b) noexcept
{
	return __a._M_pi == __b._M_pi;
}
