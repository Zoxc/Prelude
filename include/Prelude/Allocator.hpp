#pragma once
#include <cstdlib>
#include "Internal/Common.hpp"

namespace Prelude
{
	template<class T> class WithReferenceProvider
	{
		public:
			typedef WithReferenceProvider Ref;
			typedef WithReferenceProvider *Type;
					
			class Storage
			{
				private:
					T &allocator;
				public:
					Storage(Type reference) : allocator(*static_cast<T *>(reference)) {}
					Storage(const Storage &storage) : allocator(storage.allocator) {}

					Type reference()
					{
						return &allocator;
					}

					void *allocate(size_t bytes)
					{
						return allocator.allocate(bytes);
					}

					void *reallocate(void *memory, size_t old_size, size_t new_size)
					{
						return allocator.reallocate(memory, old_size, new_size);
					}

					void free(void *memory)
					{
						return allocator.free(memory);
					}
			};

			operator Type()
			{
				return this;
			}
			
			Type reference()
			{
				return this;
			}
			
			static WithReferenceProvider<T> *standard;	
	};
			
	template<class T> class NoReferenceProvider
	{
		public:
			typedef NoReferenceProvider Ref;
			typedef NoReferenceProvider *Type;
			typedef T Storage;
			
			static Type standard;
			
			typename Ref::Type reference()
			{
				return Ref::standard;
			}

	};

	template<typename T> NoReferenceProvider<T> *NoReferenceProvider<T>::standard = nullptr;

	class StandardAllocator:
		public NoReferenceProvider<StandardAllocator>
	{
		public:
			StandardAllocator(Ref::Type reference) {}
			StandardAllocator(const StandardAllocator &allocator) {}
			
			void *allocate(size_t bytes)
			{
				return std::malloc(bytes);
			}

			void *reallocate(void *memory, size_t old, size_t bytes)
			{
				return std::realloc(memory, bytes);
			}

			static const bool can_free = true;

			void free(void *memory)
			{
				return std::free(memory);
			}
	};
};
