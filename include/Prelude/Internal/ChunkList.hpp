#pragma once
#include "Common.hpp"
#include "../FastList.hpp"

namespace Prelude
{
	template<typename Allocator> class ChunkList
	{
		struct Chunk
		{
			ListEntry<Chunk> entry;
		};
		
		private:
			FastList<Chunk> chunks;
			typename Allocator::Storage allocator_reference;

		public:
			ChunkList() : allocator_reference(Allocator::Storage::default_reference()) {}
			ChunkList(typename Allocator::Reference allocator_reference) : allocator_reference(allocator_reference)	{}
			
			void *allocate(size_t bytes)
			{
				void *result = allocator_reference.allocate(bytes + sizeof(Chunk));
				
				assert(result && "No memory was allocated.");
				
				Chunk *chunk = new (result) Chunk;
				
				chunks.append(chunk);
				
				return (void *)((uint8_t *)result + sizeof(Chunk));
			}
			
			~ChunkList()
			{
				FastList<Chunk>::Iterator chunk = chunks.begin();

				while(chunk != chunks.end())
				{
					Chunk *current = *chunk;
					++chunk;
					free(current);
				};
			}
	};
};
