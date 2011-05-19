#pragma once
#include <new>
#include <stdint.h>
#include <cstdlib>
#include <cassert>

namespace Prelude
{
	static const size_t memory_align = 8;

	static inline size_t align(size_t value, size_t alignment)
	{
		alignment -= 1;
		return (value + alignment) & ~alignment;
	};
};