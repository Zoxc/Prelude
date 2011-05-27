#pragma once
#include <new>
#include <stdint.h>
#include <cstdlib>
#include <cassert>
#include <cstring>

#ifndef _MSC_VER
	const class {
	public:
		template<class T> operator T *() const
		{
		return 0;
		}
		template<class C, class T> operator T C::*() const
		{
			return 0;
		}
	private:
		void operator&() const;
	} nullptr = {};
#endif

namespace Prelude
{
	static const size_t memory_align = 8;

	static inline size_t align(size_t value, size_t alignment)
	{
		alignment -= 1;
		return (value + alignment) & ~alignment;
	};
};