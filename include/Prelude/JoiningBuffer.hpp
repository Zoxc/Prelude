#pragma once
#include <stdint.h>
#include "List.hpp"
#include "Allocator.hpp"

namespace Prelude
{
	template<size_t buffer_size, typename Allocator = Allocator::Standard> class JoiningBuffer
	{
		private:
			struct Buffer
			{
				ListEntry<Buffer> entry;
				size_t size;
			};
			
			List<Buffer> buffer_list;

			uint8_t *current;
			uint8_t *max;
			size_t bytes;
			
			Allocator allocator;

			void update()
			{
				Buffer *buffer = buffer_list.last;

				if(buffer)
					buffer->size = (size_t)current - (size_t)(buffer + 1);
			}

			void *get_buffer(size_t bytes)
			{
				prelude_debug_assert(bytes <= buffer_size);
				
				update();
				
				Buffer *result = new (allocator.allocate(buffer_size + sizeof(Buffer))) Buffer;
				
				buffer_list.append(result);
				
				current = (uint8_t *)(result + 1);
				max = current + buffer_size;
				
				return allocate(bytes);
			}
						
		public:
			JoiningBuffer(typename Allocator::Reference allocator = Allocator::default_reference) : current(0), max(0), bytes(0), allocator(allocator)
			{
			}
			
			~JoiningBuffer()
			{
				if(!Allocator::can_free)
					return;
				
				typename List<Buffer>::Iterator buffer = buffer_list.begin();

				while(buffer != buffer_list.end())
				{
					Buffer *current = *buffer;
					++buffer;
					allocator.free(current);
				};
			}
			
			size_t size()
			{
				return bytes;
			}
			
			template<typename A> void *compact(typename A::Reference ref = A::default_reference)
			{
				update();
				
				void *result = A(ref).allocate(bytes);
				
				prelude_runtime_assert(result != 0);
				
				auto pos = (uint8_t *)result;
				
				for(Buffer *buffer: buffer_list)
				{
					std::memcpy(pos, buffer + 1, buffer->size);
					pos += buffer->size;
				}
				
				return result;
			}
			
			void insert_before(JoiningBuffer &other)
			{
				if(!other.buffer_list.first)
					return;
				
				other.update();
				
				if(buffer_list.first)
				{
					other.buffer_list.last->entry.next = buffer_list.first;
					buffer_list.first = other.buffer_list.first;
				}
				else
				{
					current = other.current;
					max = other.max;
					buffer_list = other.buffer_list;
				}
				
				bytes += other.bytes;
				
				other.buffer_list.clear();
				other.current = 0;
				other.max = 0;
				other.bytes = 0;
			}
			
			void insert_after(JoiningBuffer &other)
			{
				if(!other.buffer_list.first)
					return;
				
				other.update();
				
				if(buffer_list.first)
				{
					update();
					buffer_list.last->entry.next = other.buffer_list.first;
					buffer_list.last = other.buffer_list.last;
				}
				else
					buffer_list = other.buffer_list;
					
				current = other.current;
				max = other.max;
				bytes += other.bytes;
				
				other.buffer_list.clear();
				other.current = 0;
				other.max = 0;
				other.bytes = 0;
			}
			
			void *allocate(size_t bytes)
			{
				uint8_t *result;

				result = current;

				uint8_t *next = result + bytes;
		
				if(next > max)
					return get_buffer(bytes);
				
				this->bytes += bytes;

				current = next;

				return (void *)result;
			}
	};
};
