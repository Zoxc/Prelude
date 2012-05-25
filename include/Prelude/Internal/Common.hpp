#pragma once
#include <string>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <iostream>

#ifndef __has_builtin
	#define __has_builtin(x) 0
#endif

#ifndef __has_feature
	#define __has_feature(x) 0
#endif

#if (__GNUC__ == 4) && (__GNUC_MINOR__ <= 5) && !__has_feature(cxx_nullptr)
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

#ifdef _MSC_VER
	#define WIN32 1
	#define prelude_nonnull(...)
	#define prelude_align(pre, name, value) __declspec(align(value)) pre name
	#define prelude_assume(cond) __assume(cond)	
	#define prelude_unreachable() __assume(0)	
	#define prelude_noreturn __declspec(noreturn)
	#define prelude_unused
	#define prelude_likely(x) (x)
	#define prelude_unlikely(x) (x)
	#define prelude_memory_barrier() MemoryBarrier()
	#define prelude_use_result
	#define prelude_thread __declspec(thread)
#else
	#define prelude_thread __thread
	#define prelude_nonnull(...) __attribute__((nonnull(__VA_ARGS__)))
	#define prelude_align(pre, name, value) pre __attribute__((aligned(value))) name
	#define prelude_unreachable() __builtin_unreachable()
	#define prelude_assume(cond)
	#define prelude_use_result __attribute__((warn_unused_result))
	#define prelude_memory_barrier() __sync_synchronize()
	#define prelude_unused __attribute__((unused)) 
	#define prelude_noreturn __attribute__((noreturn)) 
	#define prelude_likely(x) __builtin_expect((x),1)
	#define prelude_unlikely(x) __builtin_expect((x),0)
#endif

namespace Prelude
{
	static const size_t memory_align = 8;

	#define prelude_stringify(value) #value
	#define prelude_runtime_abort_internal(file, line, message) Prelude::runtime_abort_with_message(file ":" prelude_stringify(line) ": " + std::string(message))
	#define prelude_runtime_abort(message) prelude_runtime_abort_internal(__FILE__, __LINE__, message)
	
	static inline prelude_noreturn void runtime_abort_with_message(std::string message);
	static inline void runtime_abort_with_message(std::string message)
	{
		std::cout << message << std::endl;
		std::abort();
	}
	
	#ifdef _MSC_VER
		#define prelude_runtime_assert(expression) assert(expression)
	#else
		#define prelude_runtime_assert_internal(expression, file, line) do { if(prelude_unlikely(!(expression))) runtime_abort_with_message("Assertion failed: " #expression ", file " file ", line " prelude_stringify(line)); } while(0)
		#define prelude_runtime_assert(expression) prelude_runtime_assert_internal(expression, __FILE__, __LINE__)
	#endif

	#ifdef DEBUG
		#define prelude_debug_assert(expression) prelude_runtime_assert(expression)
		#define prelude_debug_abort(message) prelude_runtime_abort_internal(__FILE__, __LINE__, message)
	#else
		#define prelude_debug_assert(expression) prelude_assume(expression)
		#define prelude_debug_abort(message) prelude_unreachable()
	#endif

	static inline size_t align(size_t value, size_t alignment)
	{
		alignment -= 1;
		return (value + alignment) & ~alignment;
	};
	
	static inline size_t align_down(size_t value, size_t alignment)
	{
		return value & ~(alignment - 1);
	};
};
