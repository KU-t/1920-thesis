#include "shared_count.h"

template<typename _Ptr>
inline LFSP::__shared_count::__shared_count(_Ptr __p)
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