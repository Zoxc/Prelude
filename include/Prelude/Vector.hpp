#pragma once
#include "Internal/Common.hpp"
#include "Allocator/Array.hpp"

namespace Prelude
{
	template<class T, class BaseAllocator = Allocator::Standard, template<class, class> class ArrayWrapper = Allocator::Array> class Vector
	{
		protected:
			typedef ArrayWrapper<T, BaseAllocator> Allocator;
			
			typename Allocator::Storage table;
			Allocator allocator;
			size_t _size;
			size_t _capacity;

			template<class Bother, template<class, class> class Aother> void initialize_copy(const Vector<T, Bother, Aother>& other)
			{
				_size = other.size();
				_capacity = other.capacity();
				
				if(_size)
				{
					table = allocator.allocate(_capacity);

					std::memcpy((void *)raw(), (void *)other.raw(), sizeof(T) * _size);
				}
				else
				{
					table = nullptr;
				}
			}

		public:
			Vector(size_t initial, typename Allocator::Reference allocator = Allocator::default_reference) : allocator(allocator)
			{
				_size = 0;
				_capacity = 1 << initial;
				
				table = this->allocator.allocate(_capacity);
			}

			Vector(typename Allocator::Reference allocator = Allocator::default_reference) : allocator(allocator)
			{
				_size = 0;
				_capacity = 0;
				table = nullptr;
			}
			
			Vector(Vector &&vector) :
				allocator(vector.allocator),
				_size(vector._size),
				_capacity(vector._capacity)
			{
			}
			
			Vector(const Vector &vector) :
				allocator(vector.allocator)
			{
				initialize_copy(vector);
			}
			
			template<class Bother, template<class, class> class Aother> Vector(const Vector<T, Bother, Aother>& other, typename Allocator::Reference allocator = Allocator::default_reference) : allocator(allocator)
			{
				initialize_copy(other);
			}
			
			~Vector()
			{
				if(table)
					allocator.free(table);
			}
			
			Vector &operator=(const Vector& other)
			{
				if(this == &other)
					return *this;
				
				if(table)
					allocator.free(table);
				
				initialize_copy(other);
				
				return *this;
			}
			
			template<class Bother, template<class, class> class Aother> Vector &operator=(const Vector<T, Bother, Aother>& other)
			{
				if(this == &other)
					return *this;
				
				if(table)
					allocator.free(table);
				
				initialize_copy(other);
				
				return *this;
			}
			
			void expand(size_t num)
			{
				if(prelude_unlikely(_size + num > _capacity))
				{
					if(table)
					{
						prelude_debug_assert(_size > 0);

						do
						{
							_capacity <<= 1;
						}
						while(_size + num > _capacity);

						table = allocator.reallocate(table, _size, _capacity);
					}
					else
					{
						_capacity = 1;

						while(_size + num > _capacity)
							_capacity <<= 1;
						
						table = allocator.allocate(_capacity);
					}
				}
			}

			template<typename F> void mark_content(F mark)
			{
				for(size_t i = 0; i < _size; ++i)
					mark(table[i]);
			}

			template<typename F> void mark(F mark)
			{
				if(table)
					mark(table);
			}

			size_t size() const
			{
				return _size;
			}
			
			size_t capacity() const
			{
				return _capacity;
			}
			
			const T &first() const
			{
				prelude_debug_assert(_size > 0);
				
				return table[0];
			}
			
			T &first()
			{
				prelude_debug_assert(_size > 0);
				
				return table[0];
			}
			
			T *raw() const
			{
				return &table[0];
			}
			
			T &last()
			{
				prelude_debug_assert(_size > 0);
				
				return &table[_size - 1];
			}
			
			T &operator [](size_t index)
			{
				prelude_debug_assert(index < _size);
				
				return table[index];
			}
			
			const T &operator [](size_t index) const
			{
				prelude_debug_assert(index < _size);
				
				return table[index];
			}
			
			void clear()
			{
				_size = 0;
				_capacity = 0;
				
				if(table)
				{
					allocator.free(table);
					table = nullptr;
				}
			}
			
			bool expand_to(size_t size, T filler)
			{
				if(_size < size)
				{
					expand(size - _size);
					
					for(size_t i = _size; i < size - 1; ++i)
						table[i] = filler;
						
					_size = size;
					
					return true;
				}
				else
					return false;
			}
			
			T shift()
			{
				T result = first();
				
				for(size_t i = 0; i < _size - 1; ++i)
					table[i] = table[i + 1];
				
				_size -= 1;
				
				allocator.null(table[_size]);

				return result;
			}
			
			void remove(size_t index)
			{
				prelude_debug_assert(index < _size);
				
				_size -= 1;

				for(size_t i = index; i < _size; ++i)
					table[i] = table[i + 1];
					
				allocator.null(table[_size]);
			}
			
			void push_entries_front(T *entries, size_t count)
			{
				expand(count);
				
				for(size_t i = _size; i-- > 0;)
					table[i + count] = table[i];

				for(size_t i = 0; i < count; ++i)
					table[i] = entries[i];
					
				_size += count;
			}
			
			void push_entries(T *entries, size_t count)
			{
				expand(count);

				for(size_t i = 0; i < count; ++i)
					table[_size + i] = entries[i];
				
				_size += count;
			}
			
			template<class Bother, template<class, class> class Aother> void push(const Vector<T, Bother, Aother>& other)
			{
				size_t count = other.size();
				
				expand(count);
				
				for(size_t i = 0; i < count; ++i)
					table[_size + i] = other[i];
				
				_size += count;
			}
			
			void push(T entry)
			{
				expand(1);

				table[_size++] = entry;
			}
			
			T pop()
			{
				prelude_debug_assert(_size);
				
				size_t new_size = _size - 1;
				
				T result = table[new_size];
				
				if(new_size == 0)
				{
					_capacity = 0;
					allocator.free(table);
					table = nullptr;
				}
				else
					allocator.null(table[new_size]);
					
				_size = new_size;
				
				return result;
			}
			
			size_t index_of(T entry)
			{
				auto result = find(entry);

				if(!result)
					return (size_t)-1;

				return (size_t)(result - raw()) / sizeof(T);
			}
			
			template<typename F> bool each(F func)
			{
				for(size_t i = 0; i < _size; ++i)
				{
					if(!func(table[i]))
						return false;
				}
				
				return true;
			}
			
			template<typename F> T find(F func, T default_value)
			{
				for(size_t i = 0; i < _size; ++i)
				{
					if(func(table[i]))
						return table[i];
				}
				
				return default_value;
			}
			
			T *find(T entry)
			{
				for(auto i = begin(); i != end(); ++i)
				{
					if(*i == entry)
						return i.position();
				}
				
				return 0;
			}
			
			class Iterator
			{
			private:
				T *current;

			public:
				Iterator(T *start) : current(start) {}
				
				bool operator ==(const Iterator &other) const 
				{
					return current == other.current;
				}
				
				bool operator !=(const Iterator &other) const 
				{
					return current != other.current;
				}
			
				T *position() const 
				{
					return current;
				}
				
				T &operator ++()
				{
					return *++current;
				}
				
				T &operator ++(int)
				{
					return *current++;
				}
				
				T &operator*() const 
				{
					return *current;
				}
				
				T &operator ()() const 
				{
					return *current;
				}
			};
			
			Iterator begin()
			{
				return Iterator(raw());
			}

			Iterator end()
			{
				return Iterator(&table[_size]);
			}
			
			class ReverseIterator
			{
			private:
				T *current;

			public:
				ReverseIterator(T *start) : current(start) {}
				
				bool operator ==(const ReverseIterator &other) const 
				{
					return current == other.current;
				}
				
				bool operator !=(const ReverseIterator &other) const 
				{
					return current != other.current;
				}
			
				T *position() const 
				{
					return current;
				}
				
				T &operator ++()
				{
					return *--current;
				}
				
				T &operator ++(int)
				{
					return *current--;
				}
				
				T &operator*() const 
				{
					return *current;
				}
				
				T &operator ()() const 
				{
					return *current;
				}
			};
			
			ReverseIterator rbegin()
			{
				return ReverseIterator(raw() + _size - 1);
			}

			ReverseIterator rend()
			{
				return ReverseIterator(raw() - 1);
			}

			size_t index_of(Iterator &iter)
			{
				return (size_t)(iter.position() - raw()) / sizeof(T);
			}
	};
};
