#include "weak_count.h"
#include "shared_count.h"
#include "count_base.h"

constexpr LFSP::__weak_count::__weak_count() noexcept
	: _M_pi(nullptr)
{ }

LFSP::__weak_count::__weak_count(const __shared_count & __r) noexcept
	: _M_pi(__r._M_pi)
{
	if (_M_pi != nullptr)
		_M_pi->_M_weak_add_ref();
}

LFSP::__weak_count::__weak_count(const __weak_count & __r) noexcept
	: _M_pi(__r._M_pi)
{
	if (_M_pi != nullptr)
		_M_pi->_M_weak_add_ref();
}

LFSP::__weak_count::__weak_count(__weak_count && __r) noexcept
	: _M_pi(__r._M_pi)
{
	__r._M_pi = nullptr;
}

LFSP::__weak_count::~__weak_count() noexcept
{
	if (_M_pi != nullptr)
		_M_pi->_M_weak_release();
}

LFSP::__weak_count & LFSP::__weak_count::operator=(const __shared_count & __r) noexcept
{
	__counted_base* __tmp = __r._M_pi;

	if (__tmp != nullptr)
		__tmp->_M_weak_add_ref();

	if (_M_pi != nullptr)
		_M_pi->_M_weak_release();

	_M_pi = __tmp;

	return *this;
}

LFSP::__weak_count & LFSP::__weak_count::operator=(const __weak_count & __r) noexcept
{
	__counted_base* __tmp = __r._M_pi;

	if (__tmp != nullptr)
		__tmp->_M_weak_add_ref();

	if (_M_pi != nullptr)
		_M_pi->_M_weak_release();

	_M_pi = __tmp;

	return *this;
}

LFSP::__weak_count & LFSP::__weak_count::operator=(__weak_count && __r) noexcept
{
	if (_M_pi != nullptr)
		_M_pi->_M_weak_release();

	_M_pi = __r._M_pi;
	__r._M_pi = nullptr;

	return *this;
}

void LFSP::__weak_count::_M_swap(__weak_count & __r) noexcept
{
	__counted_base* __tmp = __r._M_pi;
	__r._M_pi = _M_pi;
	_M_pi = __tmp;
}

long LFSP::__weak_count::_M_get_use_count() const noexcept
{
	return _M_pi != nullptr ? _M_pi->_M_get_use_count() : 0;
}

bool LFSP::operator==(const __weak_count & __a, const __weak_count & __b) noexcept
{
	return __a._M_pi == __b._M_pi;
}
