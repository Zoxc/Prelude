#pragma once
#include "Internal/Common.hpp"

namespace Prelude
{
	template<class T> class ReferenceProvider
	{
		public:
			typedef ReferenceProvider *Reference;
					
			class ReferenceClass
			{
				private:
					T &reference;
				public:
					ReferenceClass(ReferenceProvider<T> *reference) : reference(*reference->reference) {}

					void *allocate(size_t bytes)
					{
						return reference.alloc(bytes);
					}

					void *reallocate(void *memory, size_t old_size, size_t new_size)
					{
						return reference.realloc(memory, old_size, new_size);
					}

					void free(void *memory)
					{
						return reference.free(memory);
					}
			};

			T *reference;

			ReferenceProvider() {}

			void set(T *reference)
			{
				this->reference = reference;
			}
					
			class DefaultReference
			{
				public:
					static ReferenceProvider<T> *reference;
			};
					
	};
			
	template<class T> class NoReferenceProvider
	{
		private:
			NoReferenceProvider() {}
		public:
			typedef NoReferenceProvider *Reference;
			typedef T ReferenceClass;
			typedef NoReferenceProvider DefaultReference;

			static NoReferenceProvider *reference;
	};

	template<typename T> NoReferenceProvider<T> *NoReferenceProvider<T>::reference = nullptr;

	class StandardAllocator
	{
		public:
			typedef NoReferenceProvider<StandardAllocator> ReferenceProvider;

			StandardAllocator(ReferenceProvider *reference) {}
			
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
