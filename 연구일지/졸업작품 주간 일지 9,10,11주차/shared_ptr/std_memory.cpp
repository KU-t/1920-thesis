// memory standard header
#pragma once
#ifndef _MEMORY_
#define _MEMORY_
#ifndef RC_INVOKED
#include <xmemory>
#include <exception>
#include <typeinfo>
#include <type_traits>

#pragma pack(push,_CRT_PACKING)
#pragma warning(push,_STL_WARNING_LEVEL)
#pragma warning(disable: _STL_DISABLED_WARNINGS)
_STL_DISABLE_CLANG_WARNINGS
#pragma push_macro("new")
#undef new

_STD_BEGIN
// FUNCTION TEMPLATE uninitialized_copy
template<class _InIt,
	class _FwdIt> inline
	_FwdIt _Uninitialized_copy_unchecked(_InIt _First, const _InIt _Last,
		const _FwdIt _Dest, _General_ptr_iterator_tag)
{	// copy [_First, _Last) to raw [_Dest, ...), no special optimization
	_Uninitialized_backout<_FwdIt> _Backout{ _Dest };
	for (; _First != _Last; ++_First)
	{
		_Backout._Emplace_back(*_First);
	}

	return (_Backout._Release());
}

template<class _InIt,
	class _FwdIt> inline
	_FwdIt _Uninitialized_copy_unchecked(const _InIt _First, const _InIt _Last,
		const _FwdIt _Dest, _Really_trivial_ptr_iterator_tag)
{	// copy [_First, _Last) to raw [_Dest, ...), memmove optimization
	return (_Copy_memmove(_First, _Last, _Dest));
}

template<class _InIt,
	class _FwdIt> inline
	_FwdIt uninitialized_copy(const _InIt _First, const _InIt _Last, _FwdIt _Dest)
{	// copy [_First, _Last) to raw [_Dest, ...)
	_Adl_verify_range(_First, _Last);
	const auto _UFirst = _Get_unwrapped(_First);
	const auto _ULast = _Get_unwrapped(_Last);
	const auto _UDest = _Get_unwrapped_n(_Dest, _Idl_distance<_InIt>(_UFirst, _ULast));
	_Seek_wrapped(_Dest,
		_Uninitialized_copy_unchecked(_UFirst, _ULast, _UDest, _Ptr_copy_cat(_UFirst, _UDest)));
	return (_Dest);
}

#if _ITERATOR_DEBUG_ARRAY_OVERLOADS
template<class _InIt,
	class _OutTy,
	size_t _OutSize> inline
	_OutTy * uninitialized_copy(const _InIt _First, const _InIt _Last,
		_OutTy(&_Dest)[_OutSize])
{	// copy [_First, _Last) to raw [_Dest, ...)
	return (_STD uninitialized_copy(_First, _Last, _Array_iterator<_OutTy, _OutSize>(_Dest))._Unwrapped());
}
#endif /* _ITERATOR_DEBUG_ARRAY_OVERLOADS */

// FUNCTION TEMPLATE uninitialized_copy_n
// TRANSITION: _Uninitialized_copy_n_unchecked and _Uninitialized_copy_n_unchecked1 are ABI zombie names
template<class _InIt,
	class _Diff,
	class _FwdIt> inline
	_FwdIt _Uninitialized_copy_n_unchecked2(_InIt _First, _Diff _Count,
		const _FwdIt _Dest, _General_ptr_iterator_tag)
{	// copy [_First, _First + _Count) to [_Dest, ...), no special optimization
	_Uninitialized_backout<_FwdIt> _Backout{ _Dest };
	for (; 0 < _Count; --_Count, (void)++_First)
	{
		_Backout._Emplace_back(*_First);
	}

	return (_Backout._Release());
}

template<class _InIt,
	class _Diff,
	class _FwdIt> inline
	_FwdIt _Uninitialized_copy_n_unchecked2(const _InIt _First, const _Diff _Count,
		const _FwdIt _Dest, _Really_trivial_ptr_iterator_tag)
{	// copy [_First, _First + _Count) to [_Dest, ...), memmove optimization
	return (_Copy_memmove(_First, _First + _Count, _Dest));
}

template<class _InIt,
	class _Diff,
	class _FwdIt> inline
	_FwdIt uninitialized_copy_n(const _InIt _First, const _Diff _Count_raw, _FwdIt _Dest)
{	// copy [_First, _First + _Count) to [_Dest, ...)]
	_Algorithm_int_t<_Diff> _Count = _Count_raw;
	if (0 < _Count)
	{
		const auto _UFirst = _Get_unwrapped_n(_First, _Count);
		const auto _UDest = _Get_unwrapped_n(_Dest, _Count);
		_Seek_wrapped(_Dest,
			_Uninitialized_copy_n_unchecked2(_UFirst, _Count, _UDest, _Ptr_copy_cat(_UFirst, _UDest)));
	}

	return (_Dest);
}

#if _ITERATOR_DEBUG_ARRAY_OVERLOADS
template<class _InTy,
	size_t _InSize,
	class _Diff,
	class _FwdIt> inline
	_FwdIt uninitialized_copy_n(_InTy(&_First)[_InSize], const _Diff _Count_raw, _FwdIt _Dest)
{	// copy [_First, _First + _Count) to [_Dest, ...), array input
	_Algorithm_int_t<_Diff> _Count = _Count_raw;
	if (0 < _Count)
	{
		_STL_VERIFY_ARRAY_SIZE(_First, _Count);
		const auto _UDest = _Get_unwrapped_n(_Dest, _Count);
		_Seek_wrapped(_Dest,
			_Uninitialized_copy_n_unchecked2(_First, _Count, _UDest, _Ptr_copy_cat(_First, _UDest)));
	}

	return (_Dest);
}

template<class _InIt,
	class _Diff,
	class _OutTy,
	size_t _OutSize> inline
	_OutTy * uninitialized_copy_n(const _InIt _First, const _Diff _Count_raw, _OutTy(&_Dest)[_OutSize])
{	// copy [_First, _First + _Count) to [_Dest, ...), array dest
	_Algorithm_int_t<_Diff> _Count = _Count_raw;
	if (0 < _Count)
	{
		_STL_VERIFY_ARRAY_SIZE(_Dest, _Count);
		const auto _UFirst = _Get_unwrapped_n(_First, _Count);
		return (_Uninitialized_copy_n_unchecked2(_UFirst, _Count, _Dest, _Ptr_copy_cat(_UFirst, _Dest)));
	}

	return (_Dest);
}

template<class _InTy,
	size_t _InSize,
	class _Diff,
	class _OutTy,
	size_t _OutSize> inline
	_OutTy * uninitialized_copy_n(_InTy(&_First)[_InSize], const _Diff _Count_raw, _OutTy(&_Dest)[_OutSize])
{	// copy [_First, _First + _Count) to [_Dest, ...), array input/dest
	_Algorithm_int_t<_Diff> _Count = _Count_raw;
	if (0 < _Count)
	{
		_STL_VERIFY_ARRAY_SIZE(_First, _Count);
		_STL_VERIFY_ARRAY_SIZE(_Dest, _Count);
		return (_Uninitialized_copy_n_unchecked2(_First, _Count, _Dest, _Ptr_copy_cat(_First, _Dest)));
	}

	return (_Dest);
}
#endif /* _ITERATOR_DEBUG_ARRAY_OVERLOADS */

#if _HAS_CXX17
// FUNCTION TEMPLATE uninitialized_move
template<class _InIt,
	class _FwdIt> inline
	_FwdIt uninitialized_move(const _InIt _First, const _InIt _Last, _FwdIt _Dest)
{	// move [_First, _Last) to raw [_Dest, ...)
	_Adl_verify_range(_First, _Last);
	const auto _UFirst = _Get_unwrapped(_First);
	const auto _ULast = _Get_unwrapped(_Last);
	const auto _UDest = _Get_unwrapped_n(_Dest, _Idl_distance<_InIt>(_UFirst, _ULast));
	_Seek_wrapped(_Dest,
		_Uninitialized_move_unchecked(_UFirst, _ULast, _UDest));
	return (_Dest);
}

#if _ITERATOR_DEBUG_ARRAY_OVERLOADS
template<class _InIt,
	class _OutTy,
	size_t _OutSize> inline
	_OutTy * uninitialized_move(const _InIt _First, const _InIt _Last, _OutTy(&_Dest)[_OutSize])
{	// move [_First, _Last) to raw [_Dest, ...)
	return (_STD uninitialized_move(_First, _Last, _Array_iterator<_OutTy, _OutSize>(_Dest))._Unwrapped());
}
#endif /* _ITERATOR_DEBUG_ARRAY_OVERLOADS */

// FUNCTION TEMPLATE uninitialized_move_n
template<class _InIt,
	class _Diff,
	class _FwdIt> inline
	pair<_InIt, _FwdIt> _Uninitialized_move_n_unchecked1(_InIt _First, _Diff _Count,
		const _FwdIt _Dest, _General_ptr_iterator_tag)
{	// move [_First, _First + _Count) to [_Dest, ...), no special optimization
	_Uninitialized_backout<_FwdIt> _Backout{ _Dest };
	for (; 0 < _Count; --_Count, (void)++_First)
	{
		_Backout._Emplace_back(_STD move(*_First));
	}

	return (pair<_InIt, _FwdIt>(_First, _Backout._Release()));
}

template<class _InIt,
	class _Diff,
	class _FwdIt> inline
	pair<_InIt, _FwdIt> _Uninitialized_move_n_unchecked1(_InIt _First, _Diff _Count,
		_FwdIt _Dest, _Really_trivial_ptr_iterator_tag)
{	// move [_First, _First + _Count) to [_Dest, ...), memmove optimization
	if (0 < _Count)
	{
		_Dest = _Copy_memmove(_First, _First + _Count, _Dest);
		_First += _Count;
	}

	return (pair<_InIt, _FwdIt>(_First, _Dest));
}

template<class _InIt,
	class _Diff,
	class _FwdIt> inline
	pair<_InIt, _FwdIt> _Uninitialized_move_n_unchecked(const _InIt _First, const _Diff _Count, const _FwdIt _Dest)
{	// move [_First, _First + _Count) to [_Dest, ...), choose optimization
	return (_Uninitialized_move_n_unchecked1(_First, _Count,
		_Dest, _Ptr_move_cat(_First, _Dest)));
}

template<class _InIt,
	class _Diff,
	class _FwdIt> inline
	pair<_InIt, _FwdIt> uninitialized_move_n(_InIt _First, const _Diff _Count_raw, _FwdIt _Dest)
{	// move [_First, _First + _Count) to [_Dest, ...)
	_Algorithm_int_t<_Diff> _Count = _Count_raw;
	if (0 < _Count)
	{
		const auto _Result = _Uninitialized_move_n_unchecked(
			_Get_unwrapped_n(_First, _Count), _Count, _Get_unwrapped_n(_Dest, _Count));
		_Seek_wrapped(_Dest, _Result.second);
		_Seek_wrapped(_First, _Result.first);
	}

	return { _First, _Dest };
}

#if _ITERATOR_DEBUG_ARRAY_OVERLOADS
template<class _InTy,
	size_t _InSize,
	class _Diff,
	class _FwdIt> inline
	pair<_InTy *, _FwdIt> uninitialized_move_n(_InTy(&_First)[_InSize], const _Diff _Count_raw, _FwdIt _Dest)
{	// move [_First, _First + _Count) to [_Dest, ...), array input
	_Algorithm_int_t<_Diff> _Count = _Count_raw;
	if (0 < _Count)
	{
		_STL_VERIFY_ARRAY_SIZE(_First, _Count);
		const auto _Result = _Uninitialized_move_n_unchecked(_First, _Count, _Get_unwrapped_n(_Dest, _Count));
		_Seek_wrapped(_Dest, _Result.second);
		return { _Result.first, _Dest };
	}

	return { _First, _Dest };
}

template<class _InIt,
	class _Diff,
	class _OutTy,
	size_t _OutSize> inline
	pair<_InIt, _OutTy *> uninitialized_move_n(_InIt _First, const _Diff _Count_raw, _OutTy(&_Dest)[_OutSize])
{	// move [_First, _First + _Count) to [_Dest, ...), array dest
	_Algorithm_int_t<_Diff> _Count = _Count_raw;
	if (0 < _Count)
	{
		_STL_VERIFY_ARRAY_SIZE(_Dest, _Count);
		const auto _Result = _Uninitialized_move_n_unchecked(_Get_unwrapped_n(_First, _Count), _Count, _Dest);
		_Seek_wrapped(_First, _Result.first);
		return { _First, _Result.second };
	}

	return { _First, _Dest };
}

template<class _InTy,
	size_t _InSize,
	class _Diff,
	class _OutTy,
	size_t _OutSize> inline
	pair<_InTy *, _OutTy *> uninitialized_move_n(_InTy(&_First)[_InSize], const _Diff _Count_raw,
		_OutTy(&_Dest)[_OutSize])
{	// move [_First, _First + _Count) to [_Dest, ...), array input/dest
	_Algorithm_int_t<_Diff> _Count = _Count_raw;
	if (0 < _Count)
	{
		_STL_VERIFY_ARRAY_SIZE(_First, _Count);
		_STL_VERIFY_ARRAY_SIZE(_Dest, _Count);
		return (_Uninitialized_move_n_unchecked(_First, _Count, _Dest));
	}

	return { _First, _Dest };
}
#endif /* _ITERATOR_DEBUG_ARRAY_OVERLOADS */

#endif /* _HAS_CXX17 */

// FUNCTION TEMPLATE uninitialized_fill
template<class _FwdIt,
	class _Tval> inline
	void _Uninitialized_fill_unchecked(const _FwdIt _First, const _FwdIt _Last, const _Tval& _Val, false_type)
{	// copy _Val throughout raw [_First, _Last), no special optimization
	_Uninitialized_backout<_FwdIt> _Backout{ _First };
	while (_Backout._Last != _Last)
	{
		_Backout._Emplace_back(_Val);
	}

	_Backout._Release();
}

template<class _FwdIt,
	class _Tval> inline
	void _Uninitialized_fill_unchecked(const _FwdIt _First, const _FwdIt _Last, const _Tval& _Val, true_type)
{	// copy _Val throughout raw [_First, _Last), memset optimization
	_CSTD memset(_First, static_cast<unsigned char>(_Val), static_cast<size_t>(_Last - _First));
}

template<class _FwdIt,
	class _Tval> inline
	void uninitialized_fill(const _FwdIt _First, const _FwdIt _Last, const _Tval& _Val)
{	// copy _Val throughout raw [_First, _Last)
	_Adl_verify_range(_First, _Last);
	const auto _UFirst = _Get_unwrapped(_First);
	_Uninitialized_fill_unchecked(_UFirst, _Get_unwrapped(_Last), _Val, _Fill_memset_is_safe(_UFirst, _Val));
}


// FUNCTION TEMPLATE uninitialized_fill_n
// TRANSITION: _Uninitialized_fill_n_unchecked is an ABI zombie name
template<class _FwdIt,
	class _Diff,
	class _Tval> inline
	_FwdIt _Uninitialized_fill_n_unchecked1(const _FwdIt _First, _Diff _Count, const _Tval& _Val, false_type)
{	// copy _Count copies of _Val to raw _First, no special optimization
	_Uninitialized_backout<_FwdIt> _Backout{ _First };
	for (; 0 < _Count; --_Count)
	{
		_Backout._Emplace_back(_Val);
	}

	return (_Backout._Release());
}

template<class _FwdIt,
	class _Diff,
	class _Tval> inline
	_FwdIt _Uninitialized_fill_n_unchecked1(const _FwdIt _First, const _Diff _Count, const _Tval& _Val, true_type)
{	// copy _Count copies of _Val to raw _First, memset optimization
	_CSTD memset(_First, static_cast<unsigned char>(_Val), _Count);
	return (_First + _Count);
}

template<class _FwdIt,
	class _Diff,
	class _Tval> inline
	_FwdIt uninitialized_fill_n(_FwdIt _First, const _Diff _Count_raw, const _Tval& _Val)
{	// copy _Count copies of _Val to raw _First
	_Algorithm_int_t<_Diff> _Count = _Count_raw;
	if (0 < _Count)
	{
		const auto _UFirst = _Get_unwrapped_n(_First, _Count);
		_Seek_wrapped(_First,
			_Uninitialized_fill_n_unchecked1(_UFirst, _Count, _Val, _Fill_memset_is_safe(_UFirst, _Val)));
	}

	return (_First);
}

#if _HAS_CXX17
// FUNCTION TEMPLATE destroy_at
template<class _Ty> inline
void destroy_at(_Ty * const _Location)
{	// destroy _Ty at memory address _Location
	_Location->~_Ty();
}

// FUNCTION TEMPLATE destroy
template<class _FwdIt> inline
void destroy(const _FwdIt _First, const _FwdIt _Last)
{	// destroy all elements in [_First, _Last)
	_Adl_verify_range(_First, _Last);
	_Destroy_range(_Get_unwrapped(_First), _Get_unwrapped(_Last));
}

// FUNCTION TEMPLATE destroy_n
template<class _FwdIt,
	class _Diff> inline
	_FwdIt _Destroy_n1(_FwdIt _First, _Diff _Count, false_type)
{	// destroy [_First, _First + _Count), no special optimization
	for (; 0 < _Count; ++_First, (void)--_Count)
	{
		_Destroy_in_place(*_First);
	}

	return (_First);
}

template<class _FwdIt,
	class _Diff> inline
	_FwdIt _Destroy_n1(const _FwdIt _First, const _Diff _Count, true_type)
{	// destroy [_First, _First + _Count), trivially destructible
	return (_STD next(_First, _Count)); // nothing to do
}

template<class _FwdIt,
	class _Diff> inline
	_FwdIt destroy_n(_FwdIt _First, const _Diff _Count_raw)
{	// destroy all elements in [_First, _First + _Count)
	const _Algorithm_int_t<_Diff> _Count = _Count_raw;
	if (0 < _Count)
	{
		const auto _UFirst = _Get_unwrapped_n(_First, _Count);
		_Seek_wrapped(_First, _Destroy_n1(_UFirst, _Count,
			is_trivially_destructible<_Iter_value_t<_FwdIt>>()));
	}

	return (_First);
}

// FUNCTION TEMPLATE uninitialized_default_construct
template<class _FwdIt> inline
void _Uninitialized_default_construct_unchecked(const _FwdIt _First, const _FwdIt _Last, false_type)
{	// default-initialize all elements in [_First, _Last), no special optimization
	_Uninitialized_backout<_FwdIt> _Backout{ _First };
	for (; _Backout._Last != _Last; ++_Backout._Last)
	{
		::new (static_cast<void *>(_Unfancy(_Backout._Last))) _Iter_value_t<_FwdIt>;
	}

	_Backout._Release();
}

template<class _FwdIt> inline
void _Uninitialized_default_construct_unchecked(_FwdIt, _FwdIt, true_type)
{	// default-initialize all elements in [_First, _Last), trivially default constructible types
	// nothing to do
}

template<class _FwdIt> inline
void uninitialized_default_construct(const _FwdIt _First, const _FwdIt _Last)
{	// default-initialize all elements in [_First, _Last)
	_Adl_verify_range(_First, _Last);
	_Uninitialized_default_construct_unchecked(_Get_unwrapped(_First), _Get_unwrapped(_Last),
		is_trivially_default_constructible<_Iter_value_t<_FwdIt>>());
}

// FUNCTION TEMPLATE uninitialized_default_construct_n
template<class _FwdIt,
	class _Diff> inline
	_FwdIt _Uninitialized_default_construct_n_unchecked(const _FwdIt _First, _Diff _Count, false_type)
{	// default-initialize all elements in [_First, _First + _Count), no special optimization
	_Uninitialized_backout<_FwdIt> _Backout{ _First };
	for (; 0 < _Count; ++_Backout._Last, (void)--_Count)
	{
		::new (static_cast<void *>(_Unfancy(_Backout._Last))) _Iter_value_t<_FwdIt>;
	}

	return (_Backout._Release());
}

template<class _FwdIt,
	class _Diff> inline
	_FwdIt _Uninitialized_default_construct_n_unchecked(const _FwdIt _First, const _Diff _Count, true_type)
{	// default-initialize all elements in [_First, _First + _Count), trivially default constructible types
	// nothing to do
	return (_STD next(_First, _Count));
}

template<class _FwdIt,
	class _Diff> inline
	_FwdIt uninitialized_default_construct_n(_FwdIt _First, const _Diff _Count_raw)
{	// default-initialize all elements in [_First, _First + _Count_raw)
	const _Algorithm_int_t<_Diff> _Count = _Count_raw;
	if (0 < _Count)
	{
		const auto _UFirst = _Get_unwrapped_n(_First, _Count);
		_Seek_wrapped(_First,
			_Uninitialized_default_construct_n_unchecked(_UFirst, _Count,
				is_trivially_default_constructible<_Iter_value_t<_FwdIt>>()));
	}

	return (_First);
}

// FUNCTION TEMPLATE uninitialized_value_construct
template<class _FwdIt> inline
void _Uninitialized_value_construct_unchecked(const _FwdIt _First, const _FwdIt _Last, false_type)
{	// value-initialize all elements in [_First, _Last), no special optimization
	_Uninitialized_backout<_FwdIt> _Backout{ _First };
	while (_Backout._Last != _Last)
	{
		_Backout._Emplace_back();
	}

	_Backout._Release();
}

template<class _FwdIt> inline
void _Uninitialized_value_construct_unchecked(const _FwdIt _First, const _FwdIt _Last, true_type)
{	// value-initialize all elements in [_First, _Last), all-bits-zero type
	_Zero_range(_First, _Last);
}

template<class _FwdIt> inline
void uninitialized_value_construct(const _FwdIt _First, const _FwdIt _Last)
{	// value-initialize all elements in [_First, _Last)
	_Adl_verify_range(_First, _Last);
	const auto _UFirst = _Get_unwrapped(_First);
	const auto _ULast = _Get_unwrapped(_Last);
	_Uninitialized_value_construct_unchecked(_UFirst, _ULast,
		_Use_memset_value_construct_t<_Unwrapped_t<_FwdIt>>());
}

// FUNCTION TEMPLATE uninitialized_value_construct_n
// TRANSITION: _Uninitialized_value_construct_n_unchecked is an ABI zombie name
template<class _FwdIt,
	class _Diff> inline
	_FwdIt _Uninitialized_value_construct_n_unchecked1(const _FwdIt _First, _Diff _Count, false_type)
{	// value-initialize all elements in [_First, _First + _Count), no special optimization
	_Uninitialized_backout<_FwdIt> _Backout{ _First };
	for (; 0 < _Count; --_Count)
	{
		_Backout._Emplace_back();
	}

	return (_Backout._Release());
}

template<class _FwdIt,
	class _Diff> inline
	_FwdIt _Uninitialized_value_construct_n_unchecked1(const _FwdIt _First, const _Diff _Count, true_type)
{	// value-initialize all elements in [_First, _First + _Count), all-bits-zero type
	return (_Zero_range(_First, _First + _Count));
}

template<class _FwdIt,
	class _Diff> inline
	_FwdIt uninitialized_value_construct_n(_FwdIt _First, const _Diff _Count_raw)
{	// value-initialize all elements in [_First, _First + _Count_raw)
	const _Algorithm_int_t<_Diff> _Count = _Count_raw;
	if (0 < _Count)
	{
		const auto _UFirst = _Get_unwrapped_n(_First, _Count);
		_Seek_wrapped(_First,
			_Uninitialized_value_construct_n_unchecked1(_UFirst, _Count,
				_Use_memset_value_construct_t<_Unwrapped_n_t<_FwdIt>>()));
	}

	return (_First);
}

#endif /* _HAS_CXX17 */


// CLASS TEMPLATE raw_storage_iterator
template<class _OutIt,
	class _Ty>
	class _CXX17_DEPRECATE_RAW_STORAGE_ITERATOR raw_storage_iterator
{	// wrap stores to raw buffer as output iterator
public:
	using iterator_category = output_iterator_tag;
	using value_type = void;
	using difference_type = void;
	using pointer = void;
	using reference = void;

	explicit raw_storage_iterator(_OutIt _First)
		: _Next(_First)
	{	// construct with iterator
	}

	_NODISCARD raw_storage_iterator& operator*()
	{	// pretend to return designated value
		return (*this);
	}

	raw_storage_iterator& operator=(const _Ty& _Val)
	{	// construct value designated by stored iterator
		_Construct_in_place(*_Next, _Val);
		return (*this);
	}

	raw_storage_iterator& operator=(_Ty&& _Val)
	{	// construct value designated by stored iterator
		_Construct_in_place(*_Next, _STD move(_Val));
		return (*this);
	}

	raw_storage_iterator& operator++()
	{	// preincrement
		++_Next;
		return (*this);
	}

	raw_storage_iterator operator++(int)
	{	// postincrement
		raw_storage_iterator _Ans = *this;
		++_Next;
		return (_Ans);
	}

	_NODISCARD _OutIt base() const
	{	// return the stored iterator
		return (_Next);
	}

private:
	_OutIt _Next;	// the stored iterator
};


#if _HAS_AUTO_PTR_ETC
// CLASS TEMPLATE auto_ptr
template<class _Ty>
class auto_ptr;

template<class _Ty>
struct auto_ptr_ref
{	// proxy reference for auto_ptr copying
	explicit auto_ptr_ref(_Ty * _Right)
		: _Ref(_Right)
	{	// construct from generic pointer to auto_ptr ptr
	}

	_Ty * _Ref;	// generic pointer to auto_ptr ptr
};

template<class _Ty>
class auto_ptr
{	// wrap an object pointer to ensure destruction
public:
	typedef _Ty element_type;

	explicit auto_ptr(_Ty * _Ptr = nullptr) noexcept
		: _Myptr(_Ptr)
	{	// construct from object pointer
	}

	auto_ptr(auto_ptr& _Right) noexcept
		: _Myptr(_Right.release())
	{	// construct by assuming pointer from _Right auto_ptr
	}

	auto_ptr(auto_ptr_ref<_Ty> _Right) noexcept
	{	// construct by assuming pointer from _Right auto_ptr_ref
		_Ty * _Ptr = _Right._Ref;
		_Right._Ref = nullptr;	// release old
		_Myptr = _Ptr;	// reset this
	}

	template<class _Other>
	operator auto_ptr<_Other>() noexcept
	{	// convert to compatible auto_ptr
		return (auto_ptr<_Other>(*this));
	}

	template<class _Other>
	operator auto_ptr_ref<_Other>() noexcept
	{	// convert to compatible auto_ptr_ref
		_Other * _Cvtptr = _Myptr;	// test implicit conversion
		auto_ptr_ref<_Other> _Ans(_Cvtptr);
		_Myptr = nullptr;	// pass ownership to auto_ptr_ref
		return (_Ans);
	}

	template<class _Other>
	auto_ptr& operator=(auto_ptr<_Other>& _Right) noexcept
	{	// assign compatible _Right (assume pointer)
		reset(_Right.release());
		return (*this);
	}

	template<class _Other>
	auto_ptr(auto_ptr<_Other>& _Right) noexcept
		: _Myptr(_Right.release())
	{	// construct by assuming pointer from _Right
	}

	auto_ptr& operator=(auto_ptr& _Right) noexcept
	{	// assign compatible _Right (assume pointer)
		reset(_Right.release());
		return (*this);
	}

	auto_ptr& operator=(auto_ptr_ref<_Ty> _Right) noexcept
	{	// assign compatible _Right._Ref (assume pointer)
		_Ty * _Ptr = _Right._Ref;
		_Right._Ref = 0;	// release old
		reset(_Ptr);	// set new
		return (*this);
	}

	~auto_ptr() noexcept
	{	// destroy the object
		delete _Myptr;
	}

	_NODISCARD _Ty& operator*() const noexcept
	{	// return designated value
#if _ITERATOR_DEBUG_LEVEL == 2
		_STL_VERIFY(_Myptr, "auto_ptr not dereferencable");
#endif /* _ITERATOR_DEBUG_LEVEL == 2 */

		return (*get());
	}

	_NODISCARD _Ty * operator->() const noexcept
	{	// return pointer to class object
#if _ITERATOR_DEBUG_LEVEL == 2
		_STL_VERIFY(_Myptr, "auto_ptr not dereferencable");
#endif /* _ITERATOR_DEBUG_LEVEL == 2 */

		return (get());
	}

	_NODISCARD _Ty * get() const noexcept
	{	// return wrapped pointer
		return (_Myptr);
	}

	_Ty * release() noexcept
	{	// return wrapped pointer and give up ownership
		_Ty * _Tmp = _Myptr;
		_Myptr = nullptr;
		return (_Tmp);
	}

	void reset(_Ty * _Ptr = nullptr)
	{	// destroy designated object and store new pointer
		if (_Ptr != _Myptr)
			delete _Myptr;
		_Myptr = _Ptr;
	}

private:
	_Ty * _Myptr;	// the wrapped object pointer
};

template<>
class auto_ptr<void>
{
public:
	typedef void element_type;
};
#endif /* _HAS_AUTO_PTR_ETC */


// CLASS bad_weak_ptr
class bad_weak_ptr
	: public exception
{	// exception type for invalid use of expired weak_ptr object
public:
	bad_weak_ptr() noexcept
	{	// default construct
	}

	_NODISCARD virtual const char * __CLR_OR_THIS_CALL what() const noexcept override
	{	// return pointer to message string
		return ("bad_weak_ptr");
	}
};

// CLASS _Ref_count_base
class __declspec(novtable) _Ref_count_base
{	// common code for reference counting
private:
#ifdef _M_CEE_PURE
	virtual void _Destroy() noexcept
	{	// permanent workaround to avoid mentioning _purecall in msvcurt.lib, ptrustu.lib, or other support libs
		_STD terminate();
	}

	virtual void _Delete_this() noexcept
	{	// permanent workaround to avoid mentioning _purecall in msvcurt.lib, ptrustu.lib, or other support libs
		_STD terminate();
	}
#else /* ^^^ _M_CEE_PURE ^^^ // vvv !_M_CEE_PURE vvv */
	virtual void _Destroy() noexcept = 0;
	virtual void _Delete_this() noexcept = 0;
#endif /* _M_CEE_PURE */

	_Atomic_counter_t _Uses;
	_Atomic_counter_t _Weaks;

protected:
	_Ref_count_base()
		: _Uses(1), _Weaks(1)	// non-atomic initializations
	{	// construct
	}

public:
	virtual ~_Ref_count_base() noexcept
	{	// TRANSITION, should be non-virtual
	}

	bool _Incref_nz()
	{	// increment use count if not zero, return true if successful
		for (;;)
		{	// loop until state is known
#if _USE_INTERLOCKED_REFCOUNTING
			const _Atomic_integral_t _Count =
				static_cast<volatile _Atomic_counter_t&>(_Uses);

			if (_Count == 0)
				return (false);

			if (static_cast<_Atomic_integral_t>(_InterlockedCompareExchange(
				reinterpret_cast<volatile long *>(&_Uses),
				static_cast<long>(_Count + 1), static_cast<long>(_Count))) == _Count)
				return (true);

#else /* _USE_INTERLOCKED_REFCOUNTING */
			const _Atomic_integral_t _Count =
				_Load_atomic_counter(_Uses);

			if (_Count == 0)
				return (false);

			if (_Compare_increment_atomic_counter(_Uses, _Count))
				return (true);
#endif /* _USE_INTERLOCKED_REFCOUNTING */
		}
	}

	void _Incref()
	{	// increment use count
		_MT_INCR(_Uses);
	}

	void _Incwref()
	{	// increment weak reference count
		_MT_INCR(_Weaks);
	}

	void _Decref()
	{	// decrement use count
		if (_MT_DECR(_Uses) == 0)
		{	// destroy managed resource, decrement weak reference count
			_Destroy();
			_Decwref();
		}
	}

	void _Decwref()
	{	// decrement weak reference count
		if (_MT_DECR(_Weaks) == 0)
		{
			_Delete_this();
		}
	}

	long _Use_count() const noexcept
	{	// return use count
		return (static_cast<long>(_Get_atomic_count(_Uses)));
	}

	virtual void * _Get_deleter(const type_info&) const noexcept
	{	// return address of deleter object
		return (nullptr);
	}
};

// CLASS TEMPLATE _Ref_count
template<class _Ty>
class _Ref_count
	: public _Ref_count_base
{	// handle reference counting for pointer without deleter
public:
	explicit _Ref_count(_Ty * _Px)
		: _Ref_count_base(), _Ptr(_Px)
	{	// construct
	}

private:
	virtual void _Destroy() noexcept override
	{	// destroy managed resource
		delete _Ptr;
	}

	virtual void _Delete_this() noexcept override
	{	// destroy self
		delete this;
	}

	_Ty * _Ptr;
};

// CLASS TEMPLATE _Ref_count_resource
template<class _Resource,
	class _Dx>
	class _Ref_count_resource
	: public _Ref_count_base
{	// handle reference counting for object with deleter
public:
	_Ref_count_resource(_Resource _Px, _Dx _Dt)
		: _Ref_count_base(), _Mypair(_One_then_variadic_args_t(), _STD move(_Dt), _Px)
	{	// construct
	}

	virtual void * _Get_deleter(const type_info& _Typeid) const noexcept override
	{	// return address of deleter object
#if _HAS_STATIC_RTTI
		if (_Typeid == typeid(_Dx))
		{
			return (const_cast<_Dx *>(_STD addressof(_Mypair._Get_first())));
		}
#else /* _HAS_STATIC_RTTI */
		(void)_Typeid;
#endif /* _HAS_STATIC_RTTI */

		return (nullptr);
	}

private:
	virtual void _Destroy() noexcept override
	{	// destroy managed resource
		_Mypair._Get_first()(_Mypair._Get_second());
	}

	virtual void _Delete_this() noexcept override
	{	// destroy self
		delete this;
	}

	_Compressed_pair<_Dx, _Resource> _Mypair;
};

// CLASS TEMPLATE _Ref_count_resource_alloc
template<class _Resource,
	class _Dx,
	class _Alloc>
	class _Ref_count_resource_alloc
	: public _Ref_count_base
{	// handle reference counting for object with deleter and allocator
public:
	_Ref_count_resource_alloc(_Resource _Px, _Dx _Dt, const _Alloc& _Ax)
		: _Ref_count_base(), _Mypair(_One_then_variadic_args_t(), _STD move(_Dt),
			_One_then_variadic_args_t(), _Ax, _Px)
	{	// construct
	}

	virtual void * _Get_deleter(const type_info& _Typeid) const noexcept override
	{	// return address of deleter object
#if _HAS_STATIC_RTTI
		if (_Typeid == typeid(_Dx))
		{
			return (const_cast<_Dx *>(_STD addressof(_Mypair._Get_first())));
		}
#else /* _HAS_STATIC_RTTI */
		(void)_Typeid;
#endif /* _HAS_STATIC_RTTI */

		return (nullptr);
	}

private:
	using _Myalty = _Rebind_alloc_t<_Alloc, _Ref_count_resource_alloc>;

	virtual void _Destroy() noexcept override
	{	// destroy managed resource
		_Mypair._Get_first()(_Mypair._Get_second()._Get_second());
	}

	virtual void _Delete_this() noexcept override
	{	// destroy self
		_Myalty _Al = _Mypair._Get_second()._Get_first();
		allocator_traits<_Myalty>::destroy(_Al, this);
		_Deallocate_plain(_Al, this);
	}

	_Compressed_pair<_Dx, _Compressed_pair<_Myalty, _Resource>> _Mypair;
};

// DECLARATIONS
template<class _Ty>
struct default_delete;

template<class _Ty,
	class _Dx = default_delete<_Ty>>
	class unique_ptr;

template<class _Ty>
class shared_ptr;

template<class _Ty>
class weak_ptr;

template<class _Yty,
	class = void>
	struct _Can_enable_shared
	: false_type
{	// detect unambiguous and accessible inheritance from enable_shared_from_this
};

template<class _Yty>
struct _Can_enable_shared<_Yty, void_t<typename _Yty::_Esft_type>>
	: is_convertible<remove_cv_t<_Yty> *, typename _Yty::_Esft_type *>::type
{	// is_convertible is necessary to verify unambiguous inheritance
};

template<class _Other,
	class _Yty>
	void _Enable_shared_from_this1(const shared_ptr<_Other>& _This, _Yty * _Ptr, true_type)
{	// enable shared_from_this
	if (_Ptr && _Ptr->_Wptr.expired())
	{
		_Ptr->_Wptr = shared_ptr<remove_cv_t<_Yty>>(_This, const_cast<remove_cv_t<_Yty> *>(_Ptr));
	}
}

template<class _Other,
	class _Yty>
	void _Enable_shared_from_this1(const shared_ptr<_Other>&, _Yty *, false_type)
{	// don't enable shared_from_this
}

template<class _Other,
	class _Yty>
	void _Enable_shared_from_this(const shared_ptr<_Other>& _This, _Yty * _Ptr)
{	// possibly enable shared_from_this
	_Enable_shared_from_this1(_This, _Ptr, bool_constant<conjunction_v<
		negation<is_array<_Other>>,
		negation<is_volatile<_Yty>>,
		_Can_enable_shared<_Yty>>>{});
}

// CLASS TEMPLATE _Ptr_base
template<class _Ty>
class _Ptr_base
{	// base class for shared_ptr and weak_ptr
public:
	using element_type = remove_extent_t<_Ty>;

	_NODISCARD long use_count() const noexcept
	{	// return use count
		return (_Rep ? _Rep->_Use_count() : 0);
	}

	template<class _Ty2>
	_NODISCARD bool owner_before(const _Ptr_base<_Ty2>& _Right) const noexcept
	{	// compare addresses of manager objects
		return (_Rep < _Right._Rep);
	}

	_Ptr_base(const _Ptr_base&) = delete;
	_Ptr_base& operator=(const _Ptr_base&) = delete;

protected:
	_NODISCARD element_type * get() const noexcept
	{	// return pointer to resource
		return (_Ptr);
	}

	constexpr _Ptr_base() noexcept = default;

	~_Ptr_base() = default;

	template<class _Ty2>
	void _Move_construct_from(_Ptr_base<_Ty2>&& _Right)
	{	// implement shared_ptr's (converting) move ctor and weak_ptr's move ctor
		_Ptr = _Right._Ptr;
		_Rep = _Right._Rep;

		_Right._Ptr = nullptr;
		_Right._Rep = nullptr;
	}

	template<class _Ty2>
	void _Copy_construct_from(const shared_ptr<_Ty2>& _Other)
	{	// implement shared_ptr's (converting) copy ctor
		if (_Other._Rep)
		{
			_Other._Rep->_Incref();
		}

		_Ptr = _Other._Ptr;
		_Rep = _Other._Rep;
	}

	template<class _Ty2>
	void _Alias_construct_from(const shared_ptr<_Ty2>& _Other, element_type * _Px)
	{	// implement shared_ptr's aliasing ctor
		if (_Other._Rep)
		{
			_Other._Rep->_Incref();
		}

		_Ptr = _Px;
		_Rep = _Other._Rep;
	}

	template<class _Ty0>
	friend class weak_ptr;	// specifically, weak_ptr::lock()

	template<class _Ty2>
	bool _Construct_from_weak(const weak_ptr<_Ty2>& _Other)
	{	// implement shared_ptr's ctor from weak_ptr, and weak_ptr::lock()
		if (_Other._Rep && _Other._Rep->_Incref_nz())
		{
			_Ptr = _Other._Ptr;
			_Rep = _Other._Rep;
			return (true);
		}

		return (false);
	}

	void _Decref()
	{	// decrement reference count
		if (_Rep)
		{
			_Rep->_Decref();
		}
	}

	void _Swap(_Ptr_base& _Right) noexcept
	{	// swap pointers
		_STD swap(_Ptr, _Right._Ptr);
		_STD swap(_Rep, _Right._Rep);
	}

	void _Set_ptr_rep(element_type * _Other_ptr, _Ref_count_base * _Other_rep)
	{	// take new resource
		_Ptr = _Other_ptr;
		_Rep = _Other_rep;
	}

	template<class _Ty2>
	void _Weakly_construct_from(const _Ptr_base<_Ty2>& _Other)
	{	// implement weak_ptr's ctors
		if (_Other._Rep)
		{
			_Other._Rep->_Incwref();
		}

		_Ptr = _Other._Ptr;
		_Rep = _Other._Rep;
	}

	void _Decwref()
	{	// decrement weak reference count
		if (_Rep)
		{
			_Rep->_Decwref();
		}
	}

private:
	element_type * _Ptr{ nullptr };
	_Ref_count_base * _Rep{ nullptr };

	template<class _Ty0>
	friend class _Ptr_base;

#if _HAS_STATIC_RTTI
	template<class _Dx,
		class _Ty0>
		friend _Dx * get_deleter(const shared_ptr<_Ty0>& _Sx) noexcept;
#endif /* _HAS_STATIC_RTTI */
};

// TYPE TRAIT _Can_scalar_delete
template<class _Yty,
	class = void>
	struct _Can_scalar_delete
	: false_type
{};
template<class _Yty>
struct _Can_scalar_delete<_Yty, void_t<decltype(delete _STD declval<_Yty *>())>>
	: true_type
{};

// TYPE TRAIT _Can_array_delete
template<class _Yty,
	class = void>
	struct _Can_array_delete
	: false_type
{};
template<class _Yty>
struct _Can_array_delete<_Yty, void_t<decltype(delete[] _STD declval<_Yty *>())>>
	: true_type
{};

// TYPE TRAIT _Can_call_function_object
template<class _Fx,
	class _Arg,
	class = void>
	struct _Can_call_function_object
	: false_type
{};
template<class _Fx,
	class _Arg>
	struct _Can_call_function_object<_Fx, _Arg, void_t<decltype(_STD declval<_Fx>()(_STD declval<_Arg>()))>>
	: true_type
{};

// TYPE TRAIT _SP_convertible
template<class _Yty,
	class _Ty>
	struct _SP_convertible
	: is_convertible<_Yty *, _Ty *>::type
{};
template<class _Yty,
	class _Uty>
	struct _SP_convertible<_Yty, _Uty[]>
	: is_convertible<_Yty(*)[], _Uty(*)[]>::type
{};
template<class _Yty,
	class _Uty,
	size_t _Ext>
	struct _SP_convertible<_Yty, _Uty[_Ext]>
	: is_convertible<_Yty(*)[_Ext], _Uty(*)[_Ext]>::type
{};

// TYPE TRAIT _SP_pointer_compatible
template<class _Yty,
	class _Ty>
	struct _SP_pointer_compatible
	: is_convertible<_Yty *, _Ty *>::type
{	// N4659 [util.smartptr.shared]/5 "a pointer type Y* is said to be compatible with a pointer type T* "
	// "when either Y* is convertible to T* ..."
};
template<class _Uty,
	size_t _Ext>
	struct _SP_pointer_compatible<_Uty[_Ext], _Uty[]>
	: true_type
{	// N4659 [util.smartptr.shared]/5 "... or Y is U[N] and T is cv U[]."
};
template<class _Uty,
	size_t _Ext>
	struct _SP_pointer_compatible<_Uty[_Ext], const _Uty[]>
	: true_type
{	// N4659 [util.smartptr.shared]/5 "... or Y is U[N] and T is cv U[]."
};
template<class _Uty,
	size_t _Ext>
	struct _SP_pointer_compatible<_Uty[_Ext], volatile _Uty[]>
	: true_type
{	// N4659 [util.smartptr.shared]/5 "... or Y is U[N] and T is cv U[]."
};
template<class _Uty,
	size_t _Ext>
	struct _SP_pointer_compatible<_Uty[_Ext], const volatile _Uty[]>
	: true_type
{	// N4659 [util.smartptr.shared]/5 "... or Y is U[N] and T is cv U[]."
};

// CLASS TEMPLATE shared_ptr
template<class _Ty>
class shared_ptr
	: public _Ptr_base<_Ty>
{	// class for reference counted resource management
private:
	using _Mybase = _Ptr_base<_Ty>;

public:
	using typename _Mybase::element_type;

#if _HAS_CXX17
	using weak_type = weak_ptr<_Ty>;
#endif /* _HAS_CXX17 */

	constexpr shared_ptr() noexcept
	{	// construct empty shared_ptr
	}

	constexpr shared_ptr(nullptr_t) noexcept
	{	// construct empty shared_ptr
	}

	template<class _Ux,
		enable_if_t<conjunction_v<conditional_t<is_array_v<_Ty>, _Can_array_delete<_Ux>, _Can_scalar_delete<_Ux>>,
		_SP_convertible<_Ux, _Ty>>, int> = 0>
		explicit shared_ptr(_Ux * _Px)
	{	// construct shared_ptr object that owns _Px
		_Setp(_Px, is_array<_Ty>{});
	}

	template<class _Ux,
		class _Dx,
		enable_if_t<conjunction_v<is_move_constructible<_Dx>,
		_Can_call_function_object<_Dx&, _Ux *&>,
		_SP_convertible<_Ux, _Ty>>, int> = 0>
		shared_ptr(_Ux * _Px, _Dx _Dt)
	{	// construct with _Px, deleter
		_Setpd(_Px, _STD move(_Dt));
	}

	template<class _Ux,
		class _Dx,
		class _Alloc,
		enable_if_t<conjunction_v<is_move_constructible<_Dx>,
		_Can_call_function_object<_Dx&, _Ux *&>,
		_SP_convertible<_Ux, _Ty>>, int> = 0>
		shared_ptr(_Ux * _Px, _Dx _Dt, _Alloc _Ax)
	{	// construct with _Px, deleter, allocator
		_Setpda(_Px, _STD move(_Dt), _Ax);
	}

	template<class _Dx,
		enable_if_t<conjunction_v<is_move_constructible<_Dx>,
		_Can_call_function_object<_Dx&, nullptr_t&>
	>, int> = 0>
		shared_ptr(nullptr_t, _Dx _Dt)
	{	// construct with nullptr, deleter
		_Setpd(nullptr, _STD move(_Dt));
	}

	template<class _Dx,
		class _Alloc,
		enable_if_t<conjunction_v<is_move_constructible<_Dx>,
		_Can_call_function_object<_Dx&, nullptr_t&>
	>, int> = 0>
		shared_ptr(nullptr_t, _Dx _Dt, _Alloc _Ax)
	{	// construct with nullptr, deleter, allocator
		_Setpda(nullptr, _STD move(_Dt), _Ax);
	}

	template<class _Ty2>
	shared_ptr(const shared_ptr<_Ty2>& _Right, element_type * _Px) noexcept
	{	// construct shared_ptr object that aliases _Right
		this->_Alias_construct_from(_Right, _Px);
	}

	shared_ptr(const shared_ptr& _Other) noexcept
	{	// construct shared_ptr object that owns same resource as _Other
		this->_Copy_construct_from(_Other);
	}

	template<class _Ty2,
		enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
		shared_ptr(const shared_ptr<_Ty2>& _Other) noexcept
	{	// construct shared_ptr object that owns same resource as _Other
		this->_Copy_construct_from(_Other);
	}

	shared_ptr(shared_ptr&& _Right) noexcept
	{	// construct shared_ptr object that takes resource from _Right
		this->_Move_construct_from(_STD move(_Right));
	}

	template<class _Ty2,
		enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
		shared_ptr(shared_ptr<_Ty2>&& _Right) noexcept
	{	// construct shared_ptr object that takes resource from _Right
		this->_Move_construct_from(_STD move(_Right));
	}

	template<class _Ty2,
		enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
		explicit shared_ptr(const weak_ptr<_Ty2>& _Other)
	{	// construct shared_ptr object that owns resource *_Other
		if (!this->_Construct_from_weak(_Other))
		{
			_THROW(bad_weak_ptr{});
		}
	}

#if _HAS_AUTO_PTR_ETC
	template<class _Ty2,
		enable_if_t<is_convertible_v<_Ty2 *, _Ty *>, int> = 0>
		shared_ptr(auto_ptr<_Ty2>&& _Other)
	{	// construct shared_ptr object that owns *_Other.get()
		_Ty2 * _Px = _Other.get();
		_Set_ptr_rep_and_enable_shared(_Px, new _Ref_count<_Ty2>(_Px));
		_Other.release();
	}
#endif /* _HAS_AUTO_PTR_ETC */

	template<class _Ux,
		class _Dx,
		enable_if_t<conjunction_v<
		_SP_pointer_compatible<_Ux, _Ty>,
		is_convertible<typename unique_ptr<_Ux, _Dx>::pointer, element_type *>
	>, int> = 0>
		shared_ptr(unique_ptr<_Ux, _Dx>&& _Other)
	{	// construct from unique_ptr
		using _Fancy_t = typename unique_ptr<_Ux, _Dx>::pointer;
		using _Raw_t = typename unique_ptr<_Ux, _Dx>::element_type *;
		using _Deleter_t = conditional_t<is_reference_v<_Dx>, decltype(_STD ref(_Other.get_deleter())), _Dx>;

		const _Fancy_t _Fancy = _Other.get();

		if (_Fancy)
		{
			const _Raw_t _Raw = _Fancy;
			const auto _Rx = new _Ref_count_resource<_Fancy_t, _Deleter_t>(_Fancy, _Other.get_deleter());
			_Set_ptr_rep_and_enable_shared(_Raw, _Rx);
			_Other.release();
		}
	}

	~shared_ptr() noexcept
	{	// release resource
		this->_Decref();
	}

	shared_ptr& operator=(const shared_ptr& _Right) noexcept
	{	// assign shared ownership of resource owned by _Right
		shared_ptr(_Right).swap(*this);
		return (*this);
	}

	template<class _Ty2>
	shared_ptr& operator=(const shared_ptr<_Ty2>& _Right) noexcept
	{	// assign shared ownership of resource owned by _Right
		shared_ptr(_Right).swap(*this);
		return (*this);
	}

	shared_ptr& operator=(shared_ptr&& _Right) noexcept
	{	// take resource from _Right
		shared_ptr(_STD move(_Right)).swap(*this);
		return (*this);
	}

	template<class _Ty2>
	shared_ptr& operator=(shared_ptr<_Ty2>&& _Right) noexcept
	{	// take resource from _Right
		shared_ptr(_STD move(_Right)).swap(*this);
		return (*this);
	}

#if _HAS_AUTO_PTR_ETC
	template<class _Ty2>
	shared_ptr& operator=(auto_ptr<_Ty2>&& _Right)
	{	// assign ownership of resource pointed to by _Right
		shared_ptr(_STD move(_Right)).swap(*this);
		return (*this);
	}
#endif /* _HAS_AUTO_PTR_ETC */

	template<class _Ux,
		class _Dx>
		shared_ptr& operator=(unique_ptr<_Ux, _Dx>&& _Right)
	{	// move from unique_ptr
		shared_ptr(_STD move(_Right)).swap(*this);
		return (*this);
	}

	void swap(shared_ptr& _Other) noexcept
	{	// swap pointers
		this->_Swap(_Other);
	}

	void reset() noexcept
	{	// release resource and convert to empty shared_ptr object
		shared_ptr().swap(*this);
	}

	template<class _Ux>
	void reset(_Ux * _Px)
	{	// release, take ownership of _Px
		shared_ptr(_Px).swap(*this);
	}

	template<class _Ux,
		class _Dx>
		void reset(_Ux * _Px, _Dx _Dt)
	{	// release, take ownership of _Px, with deleter _Dt
		shared_ptr(_Px, _Dt).swap(*this);
	}

	template<class _Ux,
		class _Dx,
		class _Alloc>
		void reset(_Ux * _Px, _Dx _Dt, _Alloc _Ax)
	{	// release, take ownership of _Px, with deleter _Dt, allocator _Ax
		shared_ptr(_Px, _Dt, _Ax).swap(*this);
	}

	using _Mybase::get;

	template<class _Ty2 = _Ty,
		enable_if_t<!disjunction_v<is_array<_Ty2>, is_void<_Ty2>>, int> = 0>
		_NODISCARD _Ty2& operator*() const noexcept
	{	// return reference to resource
		return (*get());
	}

	template<class _Ty2 = _Ty,
		enable_if_t<!is_array_v<_Ty2>, int> = 0>
		_NODISCARD _Ty2 * operator->() const noexcept
	{	// return pointer to resource
		return (get());
	}

	template<class _Ty2 = _Ty,
		class _Elem = element_type,
		enable_if_t<is_array_v<_Ty2>, int> = 0>
		_NODISCARD _Elem& operator[](ptrdiff_t _Idx) const
	{	// subscript
		return (get()[_Idx]);
	}

	_NODISCARD _CXX17_DEPRECATE_SHARED_PTR_UNIQUE bool unique() const noexcept
	{	// return true if no other shared_ptr object owns this resource
		return (this->use_count() == 1);
	}

	explicit operator bool() const noexcept
	{	// test if shared_ptr object owns a resource
		return (get() != nullptr);
	}

private:
	template<class _Ux>
	void _Setp(_Ux * _Px, true_type)
	{	// take ownership of _Px
		_Setpd(_Px, default_delete<_Ux[]>{});
	}

	template<class _Ux>
	void _Setp(_Ux * _Px, false_type)
	{	// take ownership of _Px
		_TRY_BEGIN	// allocate control block and set
			_Set_ptr_rep_and_enable_shared(_Px, new _Ref_count<_Ux>(_Px));
		_CATCH_ALL	// allocation failed, delete resource
			delete _Px;
		_RERAISE;
		_CATCH_END
	}

	template<class _UxptrOrNullptr,
		class _Dx>
		void _Setpd(_UxptrOrNullptr _Px, _Dx _Dt)
	{	// take ownership of _Px, deleter _Dt
		_TRY_BEGIN	// allocate control block and set
			_Set_ptr_rep_and_enable_shared(_Px, new _Ref_count_resource<_UxptrOrNullptr, _Dx>(_Px, _STD move(_Dt)));
		_CATCH_ALL	// allocation failed, delete resource
			_Dt(_Px);
		_RERAISE;
		_CATCH_END
	}

	template<class _UxptrOrNullptr,
		class _Dx,
		class _Alloc>
		void _Setpda(_UxptrOrNullptr _Px, _Dx _Dt, _Alloc _Ax)
	{	// take ownership of _Px, deleter _Dt, allocator _Ax
		using _Refd = _Ref_count_resource_alloc<_UxptrOrNullptr, _Dx, _Alloc>;
		using _Alref_alloc = _Rebind_alloc_t<_Alloc, _Refd>;
		using _Alref_traits = allocator_traits<_Alref_alloc>;
		_Alref_alloc _Alref(_Ax);

		_TRY_BEGIN	// allocate control block and set
			const auto _Pfancy = _Alref_traits::allocate(_Alref, 1);
		_Refd * const _Pref = _Unfancy(_Pfancy);
		_TRY_BEGIN
			_Alref_traits::construct(_Alref, _Pref, _Px, _STD move(_Dt), _Ax);
		_Set_ptr_rep_and_enable_shared(_Px, _Pref);
		_CATCH_ALL
			_Alref_traits::deallocate(_Alref, _Pfancy, 1);
		_RERAISE;
		_CATCH_END
			_CATCH_ALL	// allocation failed, delete resource
			_Dt(_Px);
		_RERAISE;
		_CATCH_END
	}

	template<class _Ty0,
		class... _Types>
		friend shared_ptr<_Ty0> make_shared(_Types&&... _Args);

	template<class _Ty0,
		class _Alloc,
		class... _Types>
		friend shared_ptr<_Ty0> allocate_shared(const _Alloc& _Al_arg, _Types&&... _Args);

	template<class _Ux>
	void _Set_ptr_rep_and_enable_shared(_Ux * _Px, _Ref_count_base * _Rx)
	{	// take ownership of _Px
		this->_Set_ptr_rep(_Px, _Rx);
		_Enable_shared_from_this(*this, _Px);
	}

	void _Set_ptr_rep_and_enable_shared(nullptr_t, _Ref_count_base * _Rx)
	{	// take ownership of nullptr
		this->_Set_ptr_rep(nullptr, _Rx);
	}
};

#if _HAS_CXX17
template<class _Ty>
shared_ptr(weak_ptr<_Ty>)->shared_ptr<_Ty>;

template<class _Ty,
	class _Dx>
	shared_ptr(unique_ptr<_Ty, _Dx>)->shared_ptr<_Ty>;
#endif /* _HAS_CXX17 */

template<class _Ty1,
	class _Ty2>
	_NODISCARD bool operator==(const shared_ptr<_Ty1>& _Left, const shared_ptr<_Ty2>& _Right) noexcept
{
	return (_Left.get() == _Right.get());
}

template<class _Ty1,
	class _Ty2>
	_NODISCARD bool operator!=(const shared_ptr<_Ty1>& _Left, const shared_ptr<_Ty2>& _Right) noexcept
{
	return (_Left.get() != _Right.get());
}

template<class _Ty1,
	class _Ty2>
	_NODISCARD bool operator<(const shared_ptr<_Ty1>& _Left, const shared_ptr<_Ty2>& _Right) noexcept
{
	return (_Left.get() < _Right.get());
}

template<class _Ty1,
	class _Ty2>
	_NODISCARD bool operator>=(const shared_ptr<_Ty1>& _Left, const shared_ptr<_Ty2>& _Right) noexcept
{
	return (_Left.get() >= _Right.get());
}

template<class _Ty1,
	class _Ty2>
	_NODISCARD bool operator>(const shared_ptr<_Ty1>& _Left, const shared_ptr<_Ty2>& _Right) noexcept
{
	return (_Left.get() > _Right.get());
}

template<class _Ty1,
	class _Ty2>
	_NODISCARD bool operator<=(const shared_ptr<_Ty1>& _Left, const shared_ptr<_Ty2>& _Right) noexcept
{
	return (_Left.get() <= _Right.get());
}

template<class _Ty>
_NODISCARD bool operator==(const shared_ptr<_Ty>& _Left, nullptr_t) noexcept
{
	return (_Left.get() == nullptr);
}

template<class _Ty>
_NODISCARD bool operator==(nullptr_t, const shared_ptr<_Ty>& _Right) noexcept
{
	return (nullptr == _Right.get());
}

template<class _Ty>
_NODISCARD bool operator!=(const shared_ptr<_Ty>& _Left, nullptr_t) noexcept
{
	return (_Left.get() != nullptr);
}

template<class _Ty>
_NODISCARD bool operator!=(nullptr_t, const shared_ptr<_Ty>& _Right) noexcept
{
	return (nullptr != _Right.get());
}

template<class _Ty>
_NODISCARD bool operator<(const shared_ptr<_Ty>& _Left, nullptr_t) noexcept
{
	return (_Left.get() < static_cast<typename shared_ptr<_Ty>::element_type *>(nullptr));
}

template<class _Ty>
_NODISCARD bool operator<(nullptr_t, const shared_ptr<_Ty>& _Right) noexcept
{
	return (static_cast<typename shared_ptr<_Ty>::element_type *>(nullptr) < _Right.get());
}

template<class _Ty>
_NODISCARD bool operator>=(const shared_ptr<_Ty>& _Left, nullptr_t) noexcept
{
	return (_Left.get() >= static_cast<typename shared_ptr<_Ty>::element_type *>(nullptr));
}

template<class _Ty>
_NODISCARD bool operator>=(nullptr_t, const shared_ptr<_Ty>& _Right) noexcept
{
	return (static_cast<typename shared_ptr<_Ty>::element_type *>(nullptr) >= _Right.get());
}

template<class _Ty>
_NODISCARD bool operator>(const shared_ptr<_Ty>& _Left, nullptr_t) noexcept
{
	return (_Left.get() > static_cast<typename shared_ptr<_Ty>::element_type *>(nullptr));
}

template<class _Ty>
_NODISCARD bool operator>(nullptr_t, const shared_ptr<_Ty>& _Right) noexcept
{
	return (static_cast<typename shared_ptr<_Ty>::element_type *>(nullptr) > _Right.get());
}

template<class _Ty>
_NODISCARD bool operator<=(const shared_ptr<_Ty>& _Left, nullptr_t) noexcept
{
	return (_Left.get() <= static_cast<typename shared_ptr<_Ty>::element_type *>(nullptr));
}

template<class _Ty>
_NODISCARD bool operator<=(nullptr_t, const shared_ptr<_Ty>& _Right) noexcept
{
	return (static_cast<typename shared_ptr<_Ty>::element_type *>(nullptr) <= _Right.get());
}

template<class _Elem,
	class _Traits,
	class _Ty>
	basic_ostream<_Elem, _Traits>& operator<<(basic_ostream<_Elem, _Traits>& _Out, const shared_ptr<_Ty>& _Px)
{	// write contained pointer to stream
	return (_Out << _Px.get());
}

template<class _Ty>
void swap(shared_ptr<_Ty>& _Left, shared_ptr<_Ty>& _Right) noexcept
{	// swap _Left and _Right shared_ptrs
	_Left.swap(_Right);
}

template<class _Ty1,
	class _Ty2>
	_NODISCARD shared_ptr<_Ty1> static_pointer_cast(const shared_ptr<_Ty2>& _Other) noexcept
{	// static_cast for shared_ptr that properly respects the reference count control block
	const auto _Ptr = static_cast<typename shared_ptr<_Ty1>::element_type *>(_Other.get());
	return (shared_ptr<_Ty1>(_Other, _Ptr));
}

template<class _Ty1,
	class _Ty2>
	_NODISCARD shared_ptr<_Ty1> const_pointer_cast(const shared_ptr<_Ty2>& _Other) noexcept
{	// const_cast for shared_ptr that properly respects the reference count control block
	const auto _Ptr = const_cast<typename shared_ptr<_Ty1>::element_type *>(_Other.get());
	return (shared_ptr<_Ty1>(_Other, _Ptr));
}

template<class _Ty1,
	class _Ty2>
	_NODISCARD shared_ptr<_Ty1> reinterpret_pointer_cast(const shared_ptr<_Ty2>& _Other) noexcept
{	// reinterpret_cast for shared_ptr that properly respects the reference count control block
	const auto _Ptr = reinterpret_cast<typename shared_ptr<_Ty1>::element_type *>(_Other.get());
	return (shared_ptr<_Ty1>(_Other, _Ptr));
}

#ifdef _CPPRTTI
template<class _Ty1,
	class _Ty2>
	_NODISCARD shared_ptr<_Ty1> dynamic_pointer_cast(const shared_ptr<_Ty2>& _Other) noexcept
{	// dynamic_cast for shared_ptr that properly respects the reference count control block
	const auto _Ptr = dynamic_cast<typename shared_ptr<_Ty1>::element_type *>(_Other.get());

	if (_Ptr)
	{
		return (shared_ptr<_Ty1>(_Other, _Ptr));
	}

	return (shared_ptr<_Ty1>());
}
#else /* _CPPRTTI */
template<class _Ty1,
	class _Ty2>
	shared_ptr<_Ty1> dynamic_pointer_cast(const shared_ptr<_Ty2>&) noexcept = delete;	// requires /GR option
#endif /* _CPPRTTI */

#if _HAS_STATIC_RTTI
template<class _Dx,
	class _Ty>
	_NODISCARD _Dx * get_deleter(const shared_ptr<_Ty>& _Sx) noexcept
{	// return pointer to shared_ptr's deleter object if its type is _Dx
	if (_Sx._Rep)
	{
		return (static_cast<_Dx *>(_Sx._Rep->_Get_deleter(typeid(_Dx))));
	}

	return (nullptr);
}
#else /* _HAS_STATIC_RTTI */
template<class _Dx,
	class _Ty>
	_Dx * get_deleter(const shared_ptr<_Ty>&) noexcept = delete;	// requires static RTTI
#endif /* _HAS_STATIC_RTTI */

	// CLASS TEMPLATE _Ref_count_obj
template<class _Ty>
class _Ref_count_obj
	: public _Ref_count_base
{	// handle reference counting for object in control block, no allocator
public:
	template<class... _Types>
	explicit _Ref_count_obj(_Types&&... _Args)
		: _Ref_count_base()
	{	// construct from argument list
		::new (static_cast<void *>(&_Storage)) _Ty(_STD forward<_Types>(_Args)...);
	}

	_Ty * _Getptr()
	{	// get pointer
		return (reinterpret_cast<_Ty *>(&_Storage));
	}

private:
	virtual void _Destroy() noexcept override
	{	// destroy managed resource
		_Getptr()->~_Ty();
	}

	virtual void _Delete_this() noexcept override
	{	// destroy self
		delete this;
	}

	aligned_union_t<1, _Ty> _Storage;
};

// CLASS TEMPLATE _Ref_count_obj_alloc
template<class _Ty,
	class _Alloc>
	class _Ref_count_obj_alloc
	: public _Ref_count_base
{	// handle reference counting for object in control block, allocator
public:
	template<class... _Types>
	explicit _Ref_count_obj_alloc(const _Alloc& _Al_arg, _Types&&... _Args)
		: _Ref_count_base(), _Mypair(_One_then_variadic_args_t(), _Al_arg)
	{	// construct from argument list, allocator
		::new (static_cast<void *>(&_Mypair._Get_second())) _Ty(_STD forward<_Types>(_Args)...);
	}

	_Ty * _Getptr()
	{	// get pointer
		return (reinterpret_cast<_Ty *>(&_Mypair._Get_second()));
	}

private:
	using _Myalty = _Rebind_alloc_t<_Alloc, _Ref_count_obj_alloc>;
	using _Mystoragety = aligned_union_t<1, _Ty>;

	virtual void _Destroy() noexcept override
	{	// destroy managed resource
		_Getptr()->~_Ty();
	}

	virtual void _Delete_this() noexcept override
	{	// destroy self
		_Myalty _Al = _Mypair._Get_first();
		allocator_traits<_Myalty>::destroy(_Al, this);
		_Deallocate_plain(_Al, this);
	}

	_Compressed_pair<_Myalty, _Mystoragety> _Mypair;
};

// FUNCTION TEMPLATE make_shared
template<class _Ty,
	class... _Types>
	_NODISCARD inline shared_ptr<_Ty> make_shared(_Types&&... _Args)
{	// make a shared_ptr
	const auto _Rx = new _Ref_count_obj<_Ty>(_STD forward<_Types>(_Args)...);

	shared_ptr<_Ty> _Ret;
	_Ret._Set_ptr_rep_and_enable_shared(_Rx->_Getptr(), _Rx);
	return (_Ret);
}

// FUNCTION TEMPLATE allocate_shared
template<class _Ty,
	class _Alloc,
	class... _Types>
	_NODISCARD inline shared_ptr<_Ty> allocate_shared(const _Alloc& _Al_arg, _Types&&... _Args)
{	// make a shared_ptr
	using _Refoa = _Ref_count_obj_alloc<_Ty, _Alloc>;
	using _Alref_alloc = _Rebind_alloc_t<_Alloc, _Refoa>;
	using _Alref_traits = allocator_traits<_Alref_alloc>;
	_Alref_alloc _Alref(_Al_arg);

	const auto _Rx = _Alref_traits::allocate(_Alref, 1);

	_TRY_BEGIN
		_Alref_traits::construct(_Alref, _Unfancy(_Rx), _Al_arg, _STD forward<_Types>(_Args)...);
	_CATCH_ALL
		_Alref_traits::deallocate(_Alref, _Rx, 1);
	_RERAISE;
	_CATCH_END

		shared_ptr<_Ty> _Ret;
	_Ret._Set_ptr_rep_and_enable_shared(_Rx->_Getptr(), _Unfancy(_Rx));
	return (_Ret);
}

// CLASS TEMPLATE weak_ptr
template<class _Ty>
class weak_ptr
	: public _Ptr_base<_Ty>
{	// class for pointer to reference counted resource
public:
	constexpr weak_ptr() noexcept
	{	// construct empty weak_ptr object
	}

	weak_ptr(const weak_ptr& _Other) noexcept
	{	// construct weak_ptr object for resource pointed to by _Other
		this->_Weakly_construct_from(_Other);
	}

	template<class _Ty2,
		enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
		weak_ptr(const shared_ptr<_Ty2>& _Other) noexcept
	{	// construct weak_ptr object for resource owned by _Other
		this->_Weakly_construct_from(_Other);
	}

	template<class _Ty2,
		enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
		weak_ptr(const weak_ptr<_Ty2>& _Other) noexcept
	{	// construct weak_ptr object for resource pointed to by _Other
		this->_Weakly_construct_from(_Other.lock());
	}

	weak_ptr(weak_ptr&& _Other) noexcept
	{	// move construct from _Other
		this->_Move_construct_from(_STD move(_Other));
	}

	template<class _Ty2,
		enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
		weak_ptr(weak_ptr<_Ty2>&& _Other) noexcept
	{	// move construct from _Other
		this->_Weakly_construct_from(_Other.lock());
		_Other.reset();
	}

	~weak_ptr() noexcept
	{	// release resource
		this->_Decwref();
	}

	weak_ptr& operator=(const weak_ptr& _Right) noexcept
	{	// assign from _Right
		weak_ptr(_Right).swap(*this);
		return (*this);
	}

	template<class _Ty2>
	weak_ptr& operator=(const weak_ptr<_Ty2>& _Right) noexcept
	{	// assign from _Right
		weak_ptr(_Right).swap(*this);
		return (*this);
	}

	weak_ptr& operator=(weak_ptr&& _Right) noexcept
	{	// move assign from _Right
		weak_ptr(_STD move(_Right)).swap(*this);
		return (*this);
	}

	template<class _Ty2>
	weak_ptr& operator=(weak_ptr<_Ty2>&& _Right) noexcept
	{	// move assign from _Right
		weak_ptr(_STD move(_Right)).swap(*this);
		return (*this);
	}

	template<class _Ty2>
	weak_ptr& operator=(const shared_ptr<_Ty2>& _Right) noexcept
	{	// assign from _Right
		weak_ptr(_Right).swap(*this);
		return (*this);
	}

	void reset() noexcept
	{	// release resource, convert to null weak_ptr object
		weak_ptr().swap(*this);
	}

	void swap(weak_ptr& _Other) noexcept
	{	// swap pointers
		this->_Swap(_Other);
	}

	_NODISCARD bool expired() const noexcept
	{	// return true if resource no longer exists
		return (this->use_count() == 0);
	}

	_NODISCARD shared_ptr<_Ty> lock() const noexcept
	{	// convert to shared_ptr
		shared_ptr<_Ty> _Ret;
		(void)_Ret._Construct_from_weak(*this);
		return (_Ret);
	}
};

#if _HAS_CXX17
template<class _Ty>
weak_ptr(shared_ptr<_Ty>)->weak_ptr<_Ty>;
#endif /* _HAS_CXX17 */

template<class _Ty>
void swap(weak_ptr<_Ty>& _Left, weak_ptr<_Ty>& _Right) noexcept
{	// swap contents of _Left and _Right
	_Left.swap(_Right);
}

// CLASS TEMPLATE enable_shared_from_this
template<class _Ty>
class enable_shared_from_this
{	// provide member functions that create shared_ptr to this
public:
	using _Esft_type = enable_shared_from_this;

	_NODISCARD shared_ptr<_Ty> shared_from_this()
	{	// return shared_ptr
		return (shared_ptr<_Ty>(_Wptr));
	}

	_NODISCARD shared_ptr<const _Ty> shared_from_this() const
	{	// return shared_ptr
		return (shared_ptr<const _Ty>(_Wptr));
	}

	_NODISCARD weak_ptr<_Ty> weak_from_this() noexcept
	{	// return weak_ptr
		return (_Wptr);
	}

	_NODISCARD weak_ptr<const _Ty> weak_from_this() const noexcept
	{	// return weak_ptr
		return (_Wptr);
	}

protected:
	constexpr enable_shared_from_this() noexcept
		: _Wptr()
	{	// construct
	}

	enable_shared_from_this(const enable_shared_from_this&) noexcept
		: _Wptr()
	{	// construct (must value-initialize _Wptr)
	}

	enable_shared_from_this& operator=(const enable_shared_from_this&) noexcept
	{	// assign (must not change _Wptr)
		return (*this);
	}

	~enable_shared_from_this() = default;

private:
	template<class _Other,
		class _Yty>
		friend void _Enable_shared_from_this1(const shared_ptr<_Other>& _This, _Yty * _Ptr, true_type);

	mutable weak_ptr<_Ty> _Wptr;
};


// CLASS TEMPLATE unique_ptr AND HELPERS

// STRUCT TEMPLATE default_delete
template<class _Ty>
struct default_delete
{	// default deleter for unique_ptr
	constexpr default_delete() noexcept = default;

	template<class _Ty2,
		enable_if_t<is_convertible_v<_Ty2 *, _Ty *>, int> = 0>
		default_delete(const default_delete<_Ty2>&) noexcept
	{	// construct from another default_delete
	}

	void operator()(_Ty * _Ptr) const noexcept
	{	// delete a pointer
		static_assert(0 < sizeof(_Ty),
			"can't delete an incomplete type");
		delete _Ptr;
	}
};

template<class _Ty>
struct default_delete<_Ty[]>
{	// default deleter for unique_ptr to array of unknown size
	constexpr default_delete() noexcept = default;

	template<class _Uty,
		enable_if_t<is_convertible_v<_Uty(*)[], _Ty(*)[]>, int> = 0>
		default_delete(const default_delete<_Uty[]>&) noexcept
	{	// construct from another default_delete
	}

	template<class _Uty,
		enable_if_t<is_convertible_v<_Uty(*)[], _Ty(*)[]>, int> = 0>
		void operator()(_Uty * _Ptr) const noexcept
	{	// delete a pointer
		static_assert(0 < sizeof(_Uty),
			"can't delete an incomplete type");
		delete[] _Ptr;
	}
};

// STRUCT TEMPLATE _Get_deleter_pointer_type
template<class _Ty,
	class _Dx_noref,
	class = void>
	struct _Get_deleter_pointer_type
{	// provide fallback
	typedef _Ty * type;
};

template<class _Ty,
	class _Dx_noref>
	struct _Get_deleter_pointer_type<_Ty, _Dx_noref, void_t<typename _Dx_noref::pointer>>
{	// get _Dx_noref::pointer
	typedef typename _Dx_noref::pointer type;
};

// CLASS TEMPLATE _Unique_ptr_base
template<class _Ty,
	class _Dx>
	class _Unique_ptr_base
{	// stores pointer and deleter
public:
	typedef remove_reference_t<_Dx> _Dx_noref;
	typedef typename _Get_deleter_pointer_type<_Ty, _Dx_noref>::type pointer;

	template<class _Ptr2,
		class _Dx2>
		_Unique_ptr_base(_Ptr2 _Ptr, _Dx2&& _Dt)
		: _Mypair(_One_then_variadic_args_t(), _STD forward<_Dx2>(_Dt), _Ptr)
	{	// construct with compatible pointer and deleter
	}

	template<class _Ptr2>
	constexpr _Unique_ptr_base(_Ptr2 _Ptr)
		: _Mypair(_Zero_then_variadic_args_t(), _Ptr)
	{	// construct with compatible pointer
	}

	_NODISCARD _Dx& get_deleter() noexcept
	{	// return reference to deleter
		return (_Mypair._Get_first());
	}

	_NODISCARD const _Dx& get_deleter() const noexcept
	{	// return const reference to deleter
		return (_Mypair._Get_first());
	}

	pointer& _Myptr() noexcept
	{	// return reference to pointer
		return (_Mypair._Get_second());
	}

	const pointer& _Myptr() const noexcept
	{	// return const reference to pointer
		return (_Mypair._Get_second());
	}

	_Compressed_pair<_Dx, pointer> _Mypair;
};

template<class _Dx2>
using _Unique_ptr_enable_default_t = enable_if_t<conjunction_v<negation<is_pointer<_Dx2>>,
	is_default_constructible<_Dx2>>, int>;

// CLASS TEMPLATE unique_ptr SCALAR
template<class _Ty,
	class _Dx>	// = default_delete<_Ty>
	class unique_ptr
	: public _Unique_ptr_base<_Ty, _Dx>
{	// non-copyable pointer to an object
public:
	typedef _Unique_ptr_base<_Ty, _Dx> _Mybase;
	typedef typename _Mybase::pointer pointer;
	typedef _Ty element_type;
	typedef _Dx deleter_type;

	using _Mybase::get_deleter;

	template<class _Dx2 = _Dx,
		_Unique_ptr_enable_default_t<_Dx2> = 0>
		constexpr unique_ptr() noexcept
		: _Mybase(pointer())
	{	// default construct
	}

	template<class _Dx2 = _Dx,
		_Unique_ptr_enable_default_t<_Dx2> = 0>
		constexpr unique_ptr(nullptr_t) noexcept
		: _Mybase(pointer())
	{	// null pointer construct
	}

	unique_ptr& operator=(nullptr_t) noexcept
	{	// assign a null pointer
		reset();
		return (*this);
	}

	template<class _Dx2 = _Dx,
		_Unique_ptr_enable_default_t<_Dx2> = 0>
		explicit unique_ptr(pointer _Ptr) noexcept
		: _Mybase(_Ptr)
	{	// construct with pointer
	}

	template<class _Dx2 = _Dx,
		enable_if_t<is_constructible_v<_Dx2, const _Dx2&>, int> = 0>
		unique_ptr(pointer _Ptr, const _Dx& _Dt) noexcept
		: _Mybase(_Ptr, _Dt)
	{	// construct with pointer and (maybe const) deleter&
	}

	template<class _Dx2 = _Dx,
		enable_if_t<conjunction_v<negation<is_reference<_Dx2>>,
		is_constructible<_Dx2, _Dx2>>, int> = 0>
		unique_ptr(pointer _Ptr, _Dx&& _Dt) noexcept
		: _Mybase(_Ptr, _STD move(_Dt))
	{	// construct by moving deleter
	}

	template<class _Dx2 = _Dx,
		enable_if_t<conjunction_v<is_reference<_Dx2>,
		is_constructible<_Dx2, remove_reference_t<_Dx2>>>, int> = 0>
		unique_ptr(pointer, remove_reference_t<_Dx>&&) = delete;

	unique_ptr(unique_ptr&& _Right) noexcept
		: _Mybase(_Right.release(),
			_STD forward<_Dx>(_Right.get_deleter()))
	{	// construct by moving _Right
	}

	template<class _Ty2,
		class _Dx2,
		enable_if_t<conjunction_v<negation<is_array<_Ty2>>,
		is_convertible<typename unique_ptr<_Ty2, _Dx2>::pointer, pointer>,
		conditional_t<is_reference_v<_Dx>, is_same<_Dx2, _Dx>, is_convertible<_Dx2, _Dx>>
	>, int> = 0>
		unique_ptr(unique_ptr<_Ty2, _Dx2>&& _Right) noexcept
		: _Mybase(_Right.release(),
			_STD forward<_Dx2>(_Right.get_deleter()))
	{	// construct by moving _Right
	}

#if _HAS_AUTO_PTR_ETC
	template<class _Ty2,
		enable_if_t<conjunction_v<is_convertible<_Ty2 *, _Ty *>,
		is_same<_Dx, default_delete<_Ty>>>, int> = 0>
		unique_ptr(auto_ptr<_Ty2>&& _Right) noexcept
		: _Mybase(_Right.release())
	{	// construct by moving _Right
	}
#endif /* _HAS_AUTO_PTR_ETC */

	template<class _Ty2,
		class _Dx2,
		enable_if_t<conjunction_v<negation<is_array<_Ty2>>,
		is_assignable<_Dx&, _Dx2>,
		is_convertible<typename unique_ptr<_Ty2, _Dx2>::pointer, pointer>
	>, int> = 0>
		unique_ptr& operator=(unique_ptr<_Ty2, _Dx2>&& _Right) noexcept
	{	// assign by moving _Right
		reset(_Right.release());
		this->get_deleter() = _STD forward<_Dx2>(_Right.get_deleter());
		return (*this);
	}

	unique_ptr& operator=(unique_ptr&& _Right) noexcept
	{	// assign by moving _Right
		if (this != _STD addressof(_Right))
		{	// different, do the move
			reset(_Right.release());
			this->get_deleter() = _STD forward<_Dx>(_Right.get_deleter());
		}
		return (*this);
	}

	void swap(unique_ptr& _Right) noexcept
	{	// swap elements
		_Swap_adl(this->_Myptr(), _Right._Myptr());
		_Swap_adl(this->get_deleter(), _Right.get_deleter());
	}

	~unique_ptr() noexcept
	{	// destroy the object
		if (get() != pointer())
		{
			this->get_deleter()(get());
		}
	}

	_NODISCARD add_lvalue_reference_t<_Ty> operator*() const
	{	// return reference to object
		return (*get());
	}

	_NODISCARD pointer operator->() const noexcept
	{	// return pointer to class object
		return (this->_Myptr());
	}

	_NODISCARD pointer get() const noexcept
	{	// return pointer to object
		return (this->_Myptr());
	}

	explicit operator bool() const noexcept
	{	// test for non-null pointer
		return (get() != pointer());
	}

	pointer release() noexcept
	{	// yield ownership of pointer
		pointer _Ans = get();
		this->_Myptr() = pointer();
		return (_Ans);
	}

	void reset(pointer _Ptr = pointer()) noexcept
	{	// establish new pointer
		pointer _Old = get();
		this->_Myptr() = _Ptr;
		if (_Old != pointer())
		{
			this->get_deleter()(_Old);
		}
	}

	unique_ptr(const unique_ptr&) = delete;
	unique_ptr& operator=(const unique_ptr&) = delete;
};

// CLASS TEMPLATE unique_ptr ARRAY
template<class _Ty,
	class _Dx>
	class unique_ptr<_Ty[], _Dx>
	: public _Unique_ptr_base<_Ty, _Dx>
{	// non-copyable pointer to an array object
public:
	typedef _Unique_ptr_base<_Ty, _Dx> _Mybase;
	typedef typename _Mybase::pointer pointer;
	typedef _Ty element_type;
	typedef _Dx deleter_type;

	using _Mybase::get_deleter;

	template<class _Dx2 = _Dx,
		_Unique_ptr_enable_default_t<_Dx2> = 0>
		constexpr unique_ptr() noexcept
		: _Mybase(pointer())
	{	// default construct
	}

	template<class _Uty,
		class _Is_nullptr = is_same<_Uty, nullptr_t>>
		using _Enable_ctor_reset = enable_if_t<
		is_same_v<_Uty, pointer>
		|| _Is_nullptr::value
		|| (is_same_v<pointer, element_type *>
			&& is_pointer_v<_Uty>
			&& is_convertible_v<
			remove_pointer_t<_Uty>(*)[],
			element_type(*)[]
			>)>;

	template<class _Uty,
		class _Dx2 = _Dx,
		_Unique_ptr_enable_default_t<_Dx2> = 0,
		class = _Enable_ctor_reset<_Uty>>
		explicit unique_ptr(_Uty _Ptr) noexcept
		: _Mybase(_Ptr)
	{	// construct with pointer
	}

	template<class _Uty,
		class _Dx2 = _Dx,
		enable_if_t<is_constructible_v<_Dx2, const _Dx2&>, int> = 0,
		class = _Enable_ctor_reset<_Uty>>
		unique_ptr(_Uty _Ptr, const _Dx& _Dt) noexcept
		: _Mybase(_Ptr, _Dt)
	{	// construct with pointer and (maybe const) deleter&
	}

	template<class _Uty,
		class _Dx2 = _Dx,
		enable_if_t<conjunction_v<negation<is_reference<_Dx2>>,
		is_constructible<_Dx2, _Dx2>>, int> = 0,
		class = _Enable_ctor_reset<_Uty>>
		unique_ptr(_Uty _Ptr, _Dx&& _Dt) noexcept
		: _Mybase(_Ptr, _STD move(_Dt))
	{	// construct by moving deleter
	}

	template<class _Uty,
		class _Dx2 = _Dx,
		enable_if_t<conjunction_v<is_reference<_Dx2>,
		is_constructible<_Dx2, remove_reference_t<_Dx2>>>, int> = 0>
		unique_ptr(_Uty, remove_reference_t<_Dx>&&) = delete;

	unique_ptr(unique_ptr&& _Right) noexcept
		: _Mybase(_Right.release(),
			_STD forward<_Dx>(_Right.get_deleter()))
	{	// construct by moving _Right
	}

	unique_ptr& operator=(unique_ptr&& _Right) noexcept
	{	// assign by moving _Right
		if (this != _STD addressof(_Right))
		{	// different, do the swap
			reset(_Right.release());
			this->get_deleter() = _STD move(_Right.get_deleter());
		}
		return (*this);
	}

	template<class _Uty,
		class _Ex,
		class _More,
		class _UP_pointer = typename unique_ptr<_Uty, _Ex>::pointer,
		class _UP_element_type = typename unique_ptr<_Uty, _Ex>::element_type>
		using _Enable_conversion = enable_if_t<conjunction_v<
		is_array<_Uty>,
		is_same<pointer, element_type *>,
		is_same<_UP_pointer, _UP_element_type *>,
		is_convertible<_UP_element_type(*)[], element_type(*)[]>,
		_More>>;

	template<class _Uty,
		class _Ex,
		class = _Enable_conversion<_Uty, _Ex,
		conditional_t<is_reference_v<_Dx>, is_same<_Ex, _Dx>, is_convertible<_Ex, _Dx>>>>
		unique_ptr(unique_ptr<_Uty, _Ex>&& _Right) noexcept
		: _Mybase(_Right.release(),
			_STD forward<_Ex>(_Right.get_deleter()))
	{	// construct by moving _Right
	}

	template<class _Uty,
		class _Ex,
		class = _Enable_conversion<_Uty, _Ex, is_assignable<_Dx&, _Ex>>>
		unique_ptr& operator=(unique_ptr<_Uty, _Ex>&& _Right) noexcept
	{	// assign by moving _Right
		reset(_Right.release());
		this->get_deleter() = _STD forward<_Ex>(_Right.get_deleter());
		return (*this);
	}

	template<class _Dx2 = _Dx,
		_Unique_ptr_enable_default_t<_Dx2> = 0>
		constexpr unique_ptr(nullptr_t) noexcept
		: _Mybase(pointer())
	{	// null pointer construct
	}

	unique_ptr& operator=(nullptr_t) noexcept
	{	// assign a null pointer
		reset();
		return (*this);
	}

	void reset(nullptr_t = nullptr) noexcept
	{	// establish new null pointer
		reset(pointer());
	}

	void swap(unique_ptr& _Right) noexcept
	{	// swap elements
		_Swap_adl(this->_Myptr(), _Right._Myptr());
		_Swap_adl(this->get_deleter(), _Right.get_deleter());
	}

	~unique_ptr() noexcept
	{	// destroy the object
		_Delete();
	}

	_NODISCARD _Ty& operator[](size_t _Idx) const
	{	// return reference to object
		return (get()[_Idx]);
	}

	_NODISCARD pointer get() const noexcept
	{	// return pointer to object
		return (this->_Myptr());
	}

	explicit operator bool() const noexcept
	{	// test for non-null pointer
		return (get() != pointer());
	}

	pointer release() noexcept
	{	// yield ownership of pointer
		pointer _Ans = get();
		this->_Myptr() = pointer();
		return (_Ans);
	}

	template<class _Uty,
		class = _Enable_ctor_reset<_Uty, false_type>>
		void reset(_Uty _Ptr) noexcept
	{	// establish new pointer
		pointer _Old = get();
		this->_Myptr() = _Ptr;
		if (_Old != pointer())
		{
			this->get_deleter()(_Old);
		}
	}

	unique_ptr(const unique_ptr&) = delete;
	unique_ptr& operator=(const unique_ptr&) = delete;

private:
	void _Delete()
	{	// delete the pointer
		if (get() != pointer())
		{
			this->get_deleter()(get());
		}
	}
};


// FUNCTION TEMPLATE make_unique
template<class _Ty,
	class... _Types,
	enable_if_t<!is_array_v<_Ty>, int> = 0>
	_NODISCARD inline unique_ptr<_Ty> make_unique(_Types&&... _Args)
{	// make a unique_ptr
	return (unique_ptr<_Ty>(new _Ty(_STD forward<_Types>(_Args)...)));
}

template<class _Ty,
	enable_if_t<is_array_v<_Ty> && extent_v<_Ty> == 0, int> = 0>
	_NODISCARD inline unique_ptr<_Ty> make_unique(size_t _Size)
{	// make a unique_ptr
	typedef remove_extent_t<_Ty> _Elem;
	return (unique_ptr<_Ty>(new _Elem[_Size]()));
}

template<class _Ty,
	class... _Types,
	enable_if_t<extent_v<_Ty> != 0, int> = 0>
	void make_unique(_Types&&...) = delete;


// FUNCTION TEMPLATE _Make_unique_alloc
template<class _Alloc>
struct _Allocator_deleter
{
	_Alloc _Al;

	using pointer = typename allocator_traits<_Alloc>::pointer;
	void operator()(pointer _Ptr) noexcept
	{	// delete the pointer
		allocator_traits<_Alloc>::destroy(_Al, _Unfancy(_Ptr));
		_Al.deallocate(_Ptr, 1);
	}
};

template<class _Alloc>
using _Unique_ptr_alloc = unique_ptr<typename _Alloc::value_type, _Allocator_deleter<_Alloc>>;

template<class _Alloc,
	class... _Args>
	_Unique_ptr_alloc<_Alloc> _Make_unique_alloc(_Alloc& _Al, _Args&&... _Vals)
{	// construct an object with an allocator and return it owned by a unique_ptr
	auto _Ptr = _Al.allocate(1);
	_TRY_BEGIN
		allocator_traits<_Alloc>::construct(_Al, _Unfancy(_Ptr), _STD forward<_Args>(_Vals)...);
	_CATCH_ALL
		_Al.deallocate(_Ptr, 1);
	_RERAISE;
	_CATCH_END

		return (_Unique_ptr_alloc<_Alloc>(_Ptr, _Allocator_deleter<_Alloc>{_Al}));
}

template<class _Ty,
	class _Dx,
	enable_if_t<_Is_swappable<_Dx>::value, int> = 0>
	void swap(unique_ptr<_Ty, _Dx>& _Left, unique_ptr<_Ty, _Dx>& _Right) noexcept
{	// swap _Left with _Right
	_Left.swap(_Right);
}

template<class _Ty1,
	class _Dx1,
	class _Ty2,
	class _Dx2>
	_NODISCARD bool operator==(const unique_ptr<_Ty1, _Dx1>& _Left, const unique_ptr<_Ty2, _Dx2>& _Right)
{	// test if unique_ptr _Left equals _Right
	return (_Left.get() == _Right.get());
}

template<class _Ty1,
	class _Dx1,
	class _Ty2,
	class _Dx2>
	_NODISCARD bool operator!=(const unique_ptr<_Ty1, _Dx1>& _Left, const unique_ptr<_Ty2, _Dx2>& _Right)
{	// test if unique_ptr _Left doesn't equal _Right
	return (!(_Left == _Right));
}

template<class _Ty1,
	class _Dx1,
	class _Ty2,
	class _Dx2>
	_NODISCARD bool operator<(const unique_ptr<_Ty1, _Dx1>& _Left, const unique_ptr<_Ty2, _Dx2>& _Right)
{	// test if unique_ptr _Left precedes _Right
	typedef typename unique_ptr<_Ty1, _Dx1>::pointer _Ptr1;
	typedef typename unique_ptr<_Ty2, _Dx2>::pointer _Ptr2;
	typedef common_type_t<_Ptr1, _Ptr2> _Common;
	return (less<_Common>()(_Left.get(), _Right.get()));
}

template<class _Ty1,
	class _Dx1,
	class _Ty2,
	class _Dx2>
	_NODISCARD bool operator>=(const unique_ptr<_Ty1, _Dx1>& _Left, const unique_ptr<_Ty2, _Dx2>& _Right)
{	// test if unique_ptr _Left doesn't precede _Right
	return (!(_Left < _Right));
}

template<class _Ty1,
	class _Dx1,
	class _Ty2,
	class _Dx2>
	_NODISCARD bool operator>(const unique_ptr<_Ty1, _Dx1>& _Left, const unique_ptr<_Ty2, _Dx2>& _Right)
{	// test if unique_ptr _Right precedes _Left
	return (_Right < _Left);
}

template<class _Ty1,
	class _Dx1,
	class _Ty2,
	class _Dx2>
	_NODISCARD bool operator<=(const unique_ptr<_Ty1, _Dx1>& _Left, const unique_ptr<_Ty2, _Dx2>& _Right)
{	// test if unique_ptr _Right doesn't precede _Left
	return (!(_Right < _Left));
}

template<class _Ty,
	class _Dx>
	_NODISCARD bool operator==(const unique_ptr<_Ty, _Dx>& _Left, nullptr_t) noexcept
{	// test if unique_ptr == nullptr
	return (!_Left);
}

template<class _Ty,
	class _Dx>
	_NODISCARD bool operator==(nullptr_t, const unique_ptr<_Ty, _Dx>& _Right) noexcept
{	// test if nullptr == unique_ptr
	return (!_Right);
}

template<class _Ty,
	class _Dx>
	_NODISCARD bool operator!=(const unique_ptr<_Ty, _Dx>& _Left, nullptr_t _Right) noexcept
{	// test if unique_ptr != nullptr
	return (!(_Left == _Right));
}

template<class _Ty,
	class _Dx>
	_NODISCARD bool operator!=(nullptr_t _Left, const unique_ptr<_Ty, _Dx>& _Right) noexcept
{	// test if nullptr != unique_ptr
	return (!(_Left == _Right));
}

template<class _Ty,
	class _Dx>
	_NODISCARD bool operator<(const unique_ptr<_Ty, _Dx>& _Left, nullptr_t _Right)
{	// test if unique_ptr < nullptr
	typedef typename unique_ptr<_Ty, _Dx>::pointer _Ptr;
	return (less<_Ptr>()(_Left.get(), _Right));
}

template<class _Ty,
	class _Dx>
	_NODISCARD bool operator<(nullptr_t _Left, const unique_ptr<_Ty, _Dx>& _Right)
{	// test if nullptr < unique_ptr
	typedef typename unique_ptr<_Ty, _Dx>::pointer _Ptr;
	return (less<_Ptr>()(_Left, _Right.get()));
}

template<class _Ty,
	class _Dx>
	_NODISCARD bool operator>=(const unique_ptr<_Ty, _Dx>& _Left, nullptr_t _Right)
{	// test if unique_ptr >= nullptr
	return (!(_Left < _Right));
}

template<class _Ty,
	class _Dx>
	_NODISCARD bool operator>=(nullptr_t _Left, const unique_ptr<_Ty, _Dx>& _Right)
{	// test if nullptr >= unique_ptr
	return (!(_Left < _Right));
}

template<class _Ty,
	class _Dx>
	_NODISCARD bool operator>(const unique_ptr<_Ty, _Dx>& _Left, nullptr_t _Right)
{	// test if unique_ptr > nullptr
	return (_Right < _Left);
}

template<class _Ty,
	class _Dx>
	_NODISCARD bool operator>(nullptr_t _Left, const unique_ptr<_Ty, _Dx>& _Right)
{	// test if nullptr > unique_ptr
	return (_Right < _Left);
}

template<class _Ty,
	class _Dx>
	_NODISCARD bool operator<=(const unique_ptr<_Ty, _Dx>& _Left, nullptr_t _Right)
{	// test if unique_ptr <= nullptr
	return (!(_Right < _Left));
}

template<class _Ty,
	class _Dx>
	_NODISCARD bool operator<=(nullptr_t _Left, const unique_ptr<_Ty, _Dx>& _Right)
{	// test if nullptr <= unique_ptr
	return (!(_Right < _Left));
}

template<class _OutTy,
	class _PxTy,
	class = void>
	struct _Can_stream_unique_ptr
	: false_type
{};
template<class _OutTy,
	class _PxTy>
	struct _Can_stream_unique_ptr<_OutTy, _PxTy, void_t<
	decltype(_STD declval<_OutTy>() << _STD declval<_PxTy>().get())>>
	: true_type
{};

template<class _Elem,
	class _Traits,
	class _Yty,
	class _Dx,
	enable_if_t<_Can_stream_unique_ptr<basic_ostream<_Elem, _Traits>&, const unique_ptr<_Yty, _Dx>&>::value, int> = 0>
	basic_ostream<_Elem, _Traits>& operator<<(basic_ostream<_Elem, _Traits>& _Out, const unique_ptr<_Yty, _Dx>& _Px)
{	// write contained pointer to stream
	_Out << _Px.get();
	return (_Out);
}

// GARBAGE COLLECTION
enum class pointer_safety {	// return codes for get_pointer_safety
	relaxed,
	preferred,
	strict
};

inline void declare_reachable(void *)
{	// increment pointer reachable count
}

template<class _Ty> inline
_Ty * undeclare_reachable(_Ty * _Ptr)
{	// decrement pointer reachable count
	return (_Ptr);
}

inline void declare_no_pointers(char *, size_t)
{	// declare region to be pointer free
}

inline void undeclare_no_pointers(char *, size_t)
{	// undeclare region to be pointer free
}

inline pointer_safety get_pointer_safety() noexcept
{	// get pointer safety status
	return (pointer_safety::relaxed);
}

// STRUCT TEMPLATE owner_less
template<class _Ty = void>
struct owner_less;	// not defined

template<class _Ty>
struct owner_less<shared_ptr<_Ty>>
{	// functor for owner_before
	_CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef shared_ptr<_Ty> first_argument_type;
	_CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef shared_ptr<_Ty> second_argument_type;
	_CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef bool result_type;

	_NODISCARD bool operator()(const shared_ptr<_Ty>& _Left, const shared_ptr<_Ty>& _Right) const noexcept
	{	// apply owner_before to operands
		return (_Left.owner_before(_Right));
	}

	_NODISCARD bool operator()(const shared_ptr<_Ty>& _Left, const weak_ptr<_Ty>& _Right) const noexcept
	{	// apply owner_before to operands
		return (_Left.owner_before(_Right));
	}

	_NODISCARD bool operator()(const weak_ptr<_Ty>& _Left, const shared_ptr<_Ty>& _Right) const noexcept
	{	// apply owner_before to operands
		return (_Left.owner_before(_Right));
	}
};

template<class _Ty>
struct owner_less<weak_ptr<_Ty>>
{	// functor for owner_before
	_CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef weak_ptr<_Ty> first_argument_type;
	_CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef weak_ptr<_Ty> second_argument_type;
	_CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef bool result_type;

	_NODISCARD bool operator()(const weak_ptr<_Ty>& _Left, const weak_ptr<_Ty>& _Right) const noexcept
	{	// apply owner_before to operands
		return (_Left.owner_before(_Right));
	}

	_NODISCARD bool operator()(const weak_ptr<_Ty>& _Left, const shared_ptr<_Ty>& _Right) const noexcept
	{	// apply owner_before to operands
		return (_Left.owner_before(_Right));
	}

	_NODISCARD bool operator()(const shared_ptr<_Ty>& _Left, const weak_ptr<_Ty>& _Right) const noexcept
	{	// apply owner_before to operands
		return (_Left.owner_before(_Right));
	}
};

template<>
struct owner_less<void>
{	// functor for owner_before
	using is_transparent = int;

	template<class _Ty,
		class _Uty>
		_NODISCARD bool operator()(const shared_ptr<_Ty>& _Left, const shared_ptr<_Uty>& _Right) const noexcept
	{	// apply owner_before to operands
		return (_Left.owner_before(_Right));
	}

	template<class _Ty,
		class _Uty>
		_NODISCARD bool operator()(const shared_ptr<_Ty>& _Left, const weak_ptr<_Uty>& _Right) const noexcept
	{	// apply owner_before to operands
		return (_Left.owner_before(_Right));
	}

	template<class _Ty,
		class _Uty>
		_NODISCARD bool operator()(const weak_ptr<_Ty>& _Left, const shared_ptr<_Uty>& _Right) const noexcept
	{	// apply owner_before to operands
		return (_Left.owner_before(_Right));
	}

	template<class _Ty,
		class _Uty>
		_NODISCARD bool operator()(const weak_ptr<_Ty>& _Left, const weak_ptr<_Uty>& _Right) const noexcept
	{	// apply owner_before to operands
		return (_Left.owner_before(_Right));
	}
};

// STRUCT TEMPLATE SPECIALIZATION hash
template<class _Ty,
	class _Dx>
	struct hash<unique_ptr<_Ty, _Dx>>
	: _Conditionally_enabled_hash<unique_ptr<_Ty, _Dx>,
	is_default_constructible_v<hash<typename unique_ptr<_Ty, _Dx>::pointer>>>
{	// hash functor
	static size_t _Do_hash(const unique_ptr<_Ty, _Dx>& _Keyval)
		_NOEXCEPT_COND(_Is_nothrow_hashable<typename unique_ptr<_Ty, _Dx>::pointer>::value) // strengthened
	{	// hash _Keyval to size_t value by pseudorandomizing transform
		return (hash<typename unique_ptr<_Ty, _Dx>::pointer>{}(_Keyval.get()));
	}
};

template<class _Ty>
struct hash<shared_ptr<_Ty>>
{	// hash functor
	_CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef shared_ptr<_Ty> argument_type;
	_CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef size_t result_type;

	_NODISCARD size_t operator()(const shared_ptr<_Ty>& _Keyval) const noexcept
	{	// hash _Keyval to size_t value by pseudorandomizing transform
		return (hash<typename shared_ptr<_Ty>::element_type *>()(_Keyval.get()));
	}
};

// FUNCTION align
inline void * align(size_t _Bound, size_t _Size, void *& _Ptr, size_t& _Space) noexcept
{	// try to carve out _Size bytes on boundary _Bound
	size_t _Off = static_cast<size_t>(reinterpret_cast<uintptr_t>(_Ptr) & (_Bound - 1));
	if (_Off != 0)
	{
		_Off = _Bound - _Off;	// number of bytes to skip
	}

	if (_Space < _Off || _Space - _Off < _Size)
	{
		return (nullptr);
	}

	// enough room, update
	_Ptr = static_cast<char *>(_Ptr) + _Off;
	_Space -= _Off;
	return (_Ptr);
}



/* SPIN LOCKS */
_EXTERN_C
_CRTIMP2_PURE void __cdecl _Lock_shared_ptr_spin_lock();
_CRTIMP2_PURE void __cdecl _Unlock_shared_ptr_spin_lock();
_END_EXTERN_C

// WRAP SPIN-LOCK
struct _Shared_ptr_spin_lock
{	// class to manage a spin lock for shared_ptr atomic operations
	_Shared_ptr_spin_lock()
	{	// lock the spin lock
		_Lock_shared_ptr_spin_lock();
	}

	~_Shared_ptr_spin_lock() noexcept
	{	// unlock the spin lock
		_Unlock_shared_ptr_spin_lock();
	}
};

template<class _Ty>
_NODISCARD inline bool atomic_is_lock_free(const shared_ptr<_Ty> *)
{	// return true if atomic operations on shared_ptr<_Ty> are lock-free
	return (false);
}

template<class _Ty>
_NODISCARD inline shared_ptr<_Ty> atomic_load_explicit(const shared_ptr<_Ty> * _Ptr,
	memory_order)
{	// load *_Ptr atomically
	_Shared_ptr_spin_lock _Lock;
	shared_ptr<_Ty> _Result = *_Ptr;
	return (_Result);
}

template<class _Ty>
_NODISCARD inline shared_ptr<_Ty> atomic_load(const shared_ptr<_Ty> * _Ptr)
{	// load *_Ptr atomically
	return (_STD atomic_load_explicit(_Ptr, memory_order_seq_cst));
}

template<class _Ty> inline
void atomic_store_explicit(shared_ptr<_Ty> * _Ptr, shared_ptr<_Ty> _Other,
	memory_order)
{	// store _Other to *_Ptr atomically
	_Shared_ptr_spin_lock _Lock;
	_Ptr->swap(_Other);
}

template<class _Ty> inline
void atomic_store(shared_ptr<_Ty> * _Ptr, shared_ptr<_Ty> _Other)
{	// store _Other to *_Ptr atomically
	_STD atomic_store_explicit(_Ptr, _STD move(_Other), memory_order_seq_cst);
}

template<class _Ty> inline
shared_ptr<_Ty> atomic_exchange_explicit(
	shared_ptr<_Ty> * _Ptr, shared_ptr<_Ty> _Other,
	memory_order)
{	// copy _Other to *_Ptr and return previous value of *_Ptr atomically
	_Shared_ptr_spin_lock _Lock;
	_Ptr->swap(_Other);
	return (_Other);
}

template<class _Ty> inline
shared_ptr<_Ty> atomic_exchange(
	shared_ptr<_Ty> * _Ptr, shared_ptr<_Ty> _Other)
{	// copy _Other to *_Ptr and return previous value of *_Ptr atomically
	return (_STD atomic_exchange_explicit(_Ptr, _STD move(_Other), memory_order_seq_cst));
}

template<class _Ty> inline
bool atomic_compare_exchange_weak_explicit(
	shared_ptr<_Ty> * _Ptr, shared_ptr<_Ty> * _Exp, shared_ptr<_Ty> _Value,
	memory_order, memory_order)
{	// atomically compare and exchange
	shared_ptr<_Ty> _Old_exp;	// destroyed outside spin lock
	_Shared_ptr_spin_lock _Lock;
	bool _Success = _Ptr->get() == _Exp->get()
		&& !_Ptr->owner_before(*_Exp)
		&& !_Exp->owner_before(*_Ptr);
	if (_Success)
		_Ptr->swap(_Value);
	else
	{	// match failed
		_Exp->swap(_Old_exp);
		*_Exp = *_Ptr;
	}
	return (_Success);
}

template<class _Ty> inline
bool atomic_compare_exchange_weak(
	shared_ptr<_Ty> * _Ptr, shared_ptr<_Ty> * _Exp,
	shared_ptr<_Ty> _Value)
{	// atomically compare and exchange
	return (_STD atomic_compare_exchange_weak_explicit(_Ptr, _Exp, _STD move(_Value),
		memory_order_seq_cst, memory_order_seq_cst));
}

template<class _Ty> inline
bool atomic_compare_exchange_strong_explicit(
	shared_ptr<_Ty> * _Ptr, shared_ptr<_Ty> * _Exp, shared_ptr<_Ty> _Value,
	memory_order, memory_order)
{	// atomically compare and exchange
	return (_STD atomic_compare_exchange_weak_explicit(_Ptr, _Exp, _STD move(_Value),
		memory_order_seq_cst, memory_order_seq_cst));
}

template<class _Ty> inline
bool atomic_compare_exchange_strong(
	shared_ptr<_Ty> * _Ptr, shared_ptr<_Ty> * _Exp,
	shared_ptr<_Ty> _Value)
{	// atomically compare and exchange
	return (_STD atomic_compare_exchange_strong_explicit(_Ptr, _Exp, _STD move(_Value),
		memory_order_seq_cst, memory_order_seq_cst));
}

#if _HAS_TR1_NAMESPACE
namespace _DEPRECATE_TR1_NAMESPACE tr1 {
	using _STD allocate_shared;
	using _STD bad_weak_ptr;
	using _STD const_pointer_cast;
	using _STD dynamic_pointer_cast;
	using _STD enable_shared_from_this;
	using _STD get_deleter;
	using _STD make_shared;
	using _STD shared_ptr;
	using _STD static_pointer_cast;
	using _STD swap;
	using _STD weak_ptr;
}
#endif /* _HAS_TR1_NAMESPACE */

_STD_END
#pragma pop_macro("new")
_STL_RESTORE_CLANG_WARNINGS
#pragma warning(pop)
#pragma pack(pop)
#endif /* RC_INVOKED */
#endif /* _MEMORY_ */

/*
 * Copyright (c) by P.J. Plauger. All rights reserved.
 * Consult your license regarding permissions and restrictions.
V6.50:0009 */
