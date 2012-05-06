#pragma once
#include "Internal/Common.hpp"
#include "Allocator/Array.hpp"

namespace Prelude
{
	template<class K, class V, typename Allocator = Allocator::Standard> class HashTableFunctions
	{
		public:
			static size_t hash_key(K key)
			{
				return (size_t)key;
			}

			static V invalid_value()
			{
				return 0;
			}

			static bool valid_key(K key)
			{
				return key != 0;
			}

			static bool valid_value(V value)
			{
				return value != 0;
			}
			
			static void verify_value(V)
			{
			}

			static bool create_value()
			{
				return false;
			}

			static V create_value(typename Allocator::Reference, K, size_t)
			{
				return 0;
			}
			
			static void free_value(typename Allocator::Reference, V)
			{
			}
			
			template<typename F> static void mark_value(V value, F mark)
			{
			}
	};

	template<class K, class V, class T, class BaseAllocator = Allocator::Standard, template<class, class> class ArrayWrapper = Allocator::Array> class HashTable
	{
		private:
			typedef ArrayWrapper<V, BaseAllocator> Allocator;
			typedef typename Allocator::Storage Table;
			
			Table table;
			Allocator allocator;
			size_t mask;
			size_t entries;

			static bool store(Table table, size_t mask, K key, V value)
			{
				T::verify_value(value);

				size_t hash = T::hash_key(key);
				size_t index = hash & mask;
				V entry = table[index];
				V tail = entry;
				
				T::verify_value(entry);

				while(T::valid_value(entry))
				{
					T::verify_value(entry);

					if(T::compare_key_value(key, hash, entry))
					{
						if(T::valid_value(tail))
						{
							V next = T::get_value_next(entry);
							T::verify_value(next);
							T::set_value_next(tail, value);
							T::set_value_next(value, next);
						}
						else
						{
							table[index] = value;
							T::set_value_next(value, T::invalid_value());
						}
						
						return true;
					}

					tail = entry;
					entry = T::get_value_next(entry);
				}
				
				T::verify_value(value);

				if(T::valid_value(tail))
					T::set_value_next(tail, value);
				else
					table[index] = value;

				T::set_value_next(value, T::invalid_value());

				return false;
			}

			void expand()
			{
				size_t size = (this->mask + 1) << 1;
				size_t mask = size - 1;

				Table table = allocator.allocate(size);
				
				if(!Allocator::null_references)
					for(size_t i = 0; i < size; ++i)
						table[i] = T::invalid_value();
				
				V *end = this->table + (this->mask + 1);
				
				for(V *slot = this->table; slot != end; ++slot)
				{
					V entry = *slot;

					while(T::valid_value(entry))
					{
						T::verify_value(entry);

						V next = T::get_value_next(entry);
						
						store(table, mask, T::get_key(entry), entry);
						
						entry = next;
					}
				}

				allocator.free(this->table);

				this->mask = mask;
				this->table = table;
			}

			void increase()
			{
				entries++;

				if(prelude_unlikely(entries > mask))
					expand();
			}

		protected:
			V* get_table()
			{
				return &table[0];
			}

			size_t get_size()
			{
				return mask + 1;
			}

		public:
			HashTable(size_t initial, typename Allocator::Reference allocator = Allocator::default_reference) : allocator(allocator)
			{
				entries = 0;
				
				size_t size = 1 << initial;
				mask = size - 1;

				table = this->allocator.allocate(size);
				
				if(!Allocator::null_references)
					memset(&table[0], 0, size * sizeof(V));
			}

			~HashTable()
			{
				if(Allocator::can_free)
				{
					V *end = &table[0] + (this->mask + 1);

					for(V *slot = &table[0]; slot != end; ++slot)
					{
						V entry = *slot;

						while(T::valid_value(entry))
						{
							V next = T::get_value_next(entry);

							T::free_value(get_allocator(), entry);

							entry = next;
						}
					}
					
					allocator.free(this->table);
				}
			}

			template<typename F> void mark_content(F mark)
			{
				for(size_t i = 0; i <= mask; ++i)
					if(T::valid_value(table[i]))
						T::mark_value(table[i], mark);
			
			}

			template<typename F> void mark(F mark)
			{
				mark(table);
			}

			V get(K key)
			{
				if(prelude_unlikely(!T::valid_key(key)))
					return 0;

				size_t hash = T::hash_key(key);
				size_t index = hash & mask;
				V entry = table[index];
				V tail = entry;

				while(T::valid_value(entry))
				{
					T::verify_value(entry);

					if(T::compare_key_value(key, hash, entry))
						return entry;

					tail = entry;
					entry = T::get_value_next(entry);
				}

				if(T::create_value())
				{
					V value = T::create_value(get_allocator(), key, hash);

					if(tail)
						T::set_value_next(tail, value);
					else
						table[index] = value;

					T::set_value_next(value, T::invalid_value());

					increase();

					return value;
				}
				else
					return T::invalid_value();
			}

			size_t get_entries()
			{
				return entries;
			}
			
			bool has(K key)
			{
				size_t hash = T::hash_key(key);
				size_t index = hash & mask;
				V entry = table[index];

				while(T::valid_value(entry))
				{
					T::verify_value(entry);

					if(T::compare_key_value(key, hash, entry))
						return true;
					
					entry = T::get_value_next(entry);
				}

				return false;
			}

			bool set(K key, V value)
			{
				bool exists = store(table, mask, key, value);

				if(!exists)
					increase();

				return exists;
			}
			
			template<typename F> void each_value(F func)
			{
				for(size_t i = 0; i <= mask; ++i)
				{
					V entry = table[i];

					while(T::valid_value(entry))
					{
						T::verify_value(entry);

						func(entry);
						
						entry = T::get_value_next(entry);
					}
				}
			}
			
			typename Allocator::Base::Reference get_allocator()
			{
				return allocator.reference();
			}
	};
};
