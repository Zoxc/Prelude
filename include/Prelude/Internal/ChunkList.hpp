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
			typename Allocator::ReferenceProvider::ReferenceClass allocator;

		public:
			ChunkList(typename Allocator::ReferenceProvider::Reference allocator = Allocator::ReferenceProvider::DefaultReference::reference) : allocator(allocator) {}
			
			void *allocate(size_t bytes)
			{
				void *result = allocator.allocate(bytes + sizeof(Chunk));
				
				assert(result && "No memory was allocated.");
				
				Chunk *chunk = new (result) Chunk;
				
				chunks.append(chunk);
				
				return (void *)((uint8_t *)result + sizeof(Chunk));
			}
			
			~ChunkList()
			{
				typename FastList<Chunk>::Iterator chunk = chunks.begin();

				while(chunk != chunks.end())
				{
					Chunk *current = *chunk;
					++chunk;
					free(current);
				};
			}
	};
};
