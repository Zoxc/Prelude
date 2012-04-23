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
	#define prelude_noreturn __declspec(noreturn)
	#define prelude_unused
	#define prelude_likely(x) (x)
	#define prelude_unlikely(x) (x)
#else
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
		std::cout << message << "\n";
		std::abort();
	}
	
	#ifdef _MSC_VER
		#define prelude_runtime_assert(expression) assert(expression)
	#else
		#define prelude_runtime_assert_internal(expression, file, line) do { if(prelude_unlikely(!(expression))) { std::cout << "Assertion failed: " #expression ", file " file ", line " prelude_stringify(line) "\n"; asm("int $3"); } } while(0)
		#define prelude_runtime_assert(expression) prelude_runtime_assert_internal(expression, __FILE__, __LINE__)
	#endif

	#ifdef DEBUG
		#define prelude_debug_assert(expression) prelude_runtime_assert(expression)
		#define prelude_debug_abort(message) prelude_runtime_abort_internal(__FILE__, __LINE__, message)
	#else
		#define prelude_debug_assert(expression)
		#define prelude_debug_abort(message)
	#endif

	#if !__has_builtin(__builtin_unreachable)
		static inline void prelude_noreturn __builtin_unreachable();
		static inline void __builtin_unreachable()
		{
			prelude_runtime_abort("Unreachable code");
		}
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
