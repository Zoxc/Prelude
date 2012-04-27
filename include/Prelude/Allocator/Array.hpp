#pragma once
#include "../Allocator.hpp"

namespace Prelude
{
	namespace Allocator
	{
		template<class A, class BaseAllocator> class Array:
			public BaseAllocator::ReferenceBase
		{
			public:
				typedef BaseAllocator Base;
				
			private:
				typedef BaseAllocator Allocator;
				
				Allocator allocator;
				
			public:
				operator typename Allocator::Reference()
				{
					return allocator.reference();
				}
				
				typename Allocator::Reference reference()
				{
					return allocator.reference();
				}
				
				Array(typename Allocator::Reference allocator = Allocator::default_reference) : allocator(allocator) {}
				Array(const Array &array_allocator) : allocator(allocator) {}
				
				typedef A *Storage;
				
				Storage allocate(size_t size)
				{
					return (A *)allocator.allocate(sizeof(A) * size);
				}
				
				Storage reallocate(const Storage &old, size_t old_size, size_t new_size)
				{
					return (A *)allocator.reallocate((void *)old, sizeof(A) * old_size, sizeof(A) * new_size);
				}

				void free(const Storage &array)
				{
					return allocator.free((void *)array);
				}
				
		};
	};
};
