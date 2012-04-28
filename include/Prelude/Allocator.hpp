#pragma once
#include <cstdlib>
#include "Internal/Common.hpp"

namespace Prelude
{
	namespace Allocator
	{
		template<class Ref, class Ent> struct BaseInfo
		{
			typedef Ent Base;
			typedef Ref *Reference;
			
			static const bool can_free = Ent::can_free;
		};

		template<class Ref, class Ent> struct ReferenceTemplateBase:
			public BaseInfo<Ref, Ent>
		{
			typedef ReferenceTemplateBase ReferenceBase;
			
			static Ref *default_reference;	
		};
		
		template<class T> class ReferenceTemplate:
			public ReferenceTemplateBase<ReferenceTemplate<T>, T>
		{
			private:
				T &allocator;
			public:
				ReferenceTemplate(T &allocator) : allocator(allocator) {}
				ReferenceTemplate(ReferenceTemplate *reference) : allocator(*(T *)(reference)) {}
				ReferenceTemplate(const ReferenceTemplate &provider) : allocator(provider.allocator) {}
				
				ReferenceTemplate *reference()
				{
					return (ReferenceTemplate *)&allocator;
				}
				
				operator ReferenceTemplate *()
				{
					return reference();
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
		
		template<class Ref, class Ent> struct TemplateBase:
			public BaseInfo<Ref, Ent>
		{
			typedef TemplateBase ReferenceBase;
			
			static Ref *default_reference;
		};

		template<class Ref, class Ent> Ref *TemplateBase<Ref, Ent>::default_reference = nullptr;

		template<class T> class Template:
			public TemplateBase<Template<T>, T>
		{
			public:
				Template(const T &) {}
				Template(Template *reference) {}
				Template(const Template &provider) {}
				
				Template *reference()
				{
					return Template::default_reference;
				}
				
				operator Template *()
				{
					return reference();
				}
				
				static void *allocate(size_t bytes)
				{
					return T::allocate(bytes);
				}

				static void *reallocate(void *memory, size_t old, size_t bytes)
				{
					return T::reallocate(memory, old, bytes);
				}

				static void free(void *memory)
				{
					return T::free(memory);
				}
		};

		struct StandardImplementation
		{
			static void *allocate(size_t bytes)
			{
				return std::malloc(bytes);
			}

			static void *reallocate(void *memory, size_t old, size_t bytes)
			{
				return std::realloc(memory, bytes);
			}

			static const bool can_free = true;

			static void free(void *memory)
			{
				return std::free(memory);
			}
		};
		
		typedef Template<StandardImplementation> Standard;
	};
};
