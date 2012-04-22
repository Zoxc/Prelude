#pragma once
#include "Internal/Common.hpp"
#include "Allocator.hpp"

namespace Prelude
{
	template<class K, class V, typename Allocator = StandardAllocator> class HashTableFunctions
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

			static bool create_value()
			{
				return false;
			}

			static V create_value(typename Allocator::Ref::Type allocator, K key)
			{
				return 0;
			}
			
			static void free_value(typename Allocator::Ref::Type allocator, V value)
			{
			}
	};

	template<class K, class V, class T, typename Allocator = StandardAllocator> class HashTable
	{
		private:
			V *table;
			typename Allocator::Ref::Storage allocator;
			size_t mask;
			size_t entries;

			bool store(V *table, size_t mask, K key, V value)
			{
				size_t index = T::hash_key(key) & mask;
				V entry = table[index];
				V tail = entry;

				while(T::valid_value(entry))
				{
					if(T::compare_key_value(key, entry))
					{
						if(T::valid_value(tail))
						{
							V next = T::get_value_next(entry);
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

				V *table = (V *)allocator.allocate(size * sizeof(V));
				std::memset(table, 0, size * sizeof(V));

				V *end = this->table + (this->mask + 1);

				for(V *slot = this->table; slot != end; ++slot)
				{
					V entry = *slot;

					while(T::valid_value(entry))
					{
						V next = T::get_value_next(entry);

						store(table, mask, T::get_key(entry), entry);

						T::free_value(get_allocator(), entry);

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
				return table;
			}

			size_t get_size()
			{
				return mask + 1;
			}

		public:
			HashTable(size_t initial, typename Allocator::Ref::Type allocator = Allocator::Ref::standard) : allocator(allocator)
			{
				entries = 0;
				
				size_t size = 1 << initial;
				mask = size - 1;

				table = (V *)this->allocator.allocate(size * sizeof(V));
				memset(table, 0, size * sizeof(V));
			}

			~HashTable()
			{
				if(Allocator::can_free)
				{
					V *end = this->table + (this->mask + 1);

					for(V *slot = this->table; slot != end; ++slot)
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

			V get(K key)
			{
				if(prelude_unlikely(!T::valid_key(key)))
					return 0;

				size_t index = T::hash_key(key) & mask;
				V entry = table[index];
				V tail = entry;

				while(T::valid_value(entry))
				{
					if(T::compare_key_value(key, entry))
						return entry;

					tail = entry;
					entry = T::get_value_next(entry);
				}

				if(T::create_value())
				{
					V value = T::create_value(get_allocator(), key);

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
				size_t index = T::hash_key(key) & mask;
				V entry = table[index];

				while(T::valid_value(entry))
				{
					if(T::compare_key_value(key, entry))
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
			
			typename Allocator::Ref::Type get_allocator()
			{
				return allocator.reference();
			}
	};
};
