#pragma once
#include <stdint.h>
#include "Internal/ChunkList.hpp"
#include "Allocator.hpp"

namespace Prelude
{
	template<typename Allocator = StandardAllocator> class Region
	{
		static const unsigned int max_alloc = 0x1000;

		private:
			ChunkList<Allocator> chunk_list;

			uint8_t *current;
			uint8_t *max;

			uint8_t *allocate_page(size_t bytes = max_alloc);

			void *get_page(size_t bytes)
			{
				if(bytes > max_alloc)
					return chunk_list.allocate(bytes);

				uint8_t *result = (uint8_t *)chunk_list.allocate(max_alloc);

				max = result + max_alloc;

				result = (uint8_t *)align((size_t)result, memory_align);

				current = result + bytes;
		
				return result;
			}
		public:
			typedef typename ReferenceProvider<Region> ReferenceProvider;

			ReferenceProvider reference;

			Region(typename Allocator::ReferenceProvider::Reference allocator = Allocator::ReferenceProvider::DefaultReference::reference) : chunk_list(allocator), current(0), max(0)
			{
				reference.set(this);
			}
			
			operator ReferenceProvider *()
			{
				return &reference;
			}

			void *allocate(size_t bytes)
			{
				uint8_t *result;

				result = (uint8_t *)align((size_t)current, memory_align);

				uint8_t *next = result + bytes;
		
				if(next > max)
					return get_page(bytes);

				current = next;

				return (void *)result;
			}
			
			void *reallocate(void *memory, size_t old_size, size_t new_size)
			{
				// TODO: Check if this was the previous memory block allocate and try to expand the area.

				void *result = allocate(new_size);
				
				std::memcpy(result, mem, old_size);
			
				return result;
			}
	
			void free(void *memory)
			{
			}
	};
};

template<typename Allocator> static inline void *operator new(size_t bytes, Prelude::Region<Allocator> &region) throw()
{
	return region.allocate(bytes);
}

template<typename Allocator> static inline void operator delete(void *, Prelude::Region<Allocator> &region) throw()
{
}

template<typename Allocator> static inline void *operator new[](size_t bytes, Prelude::Region<Allocator> &region) throw()
{
	return region.allocate(bytes);
}

template<typename Allocator> static inline void operator delete[](void *, Prelude::Region<Allocator> &region) throw()
{
}
