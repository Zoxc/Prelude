#pragma once
#include "Internal/Common.hpp"
#include "Allocator.hpp"

namespace Prelude
{
	template<class K, class V> class MapFunctions
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
	};

	template<class K, class V, typename Allocator = StandardAllocator, class T = MapFunctions<K ,V> > class Map
	{
		private:
			struct Pair
			{
				K key;
				Pair *next;
				V value;
			};

			Pair **table;
			typename Allocator::Ref::Storage allocator;
			size_t mask;
			size_t entries;
			
			bool store(Pair **table, size_t mask, K key, V value)
			{
				size_t index = T::hash_key(key) & mask;
				Pair *pair = table[index];
				Pair *tail = pair;

				while(pair)
				{
					if(pair->key == key)
					{
						pair->value = value;
						return false;
					}

					tail = pair;
					pair = pair->next;
				}
				
				pair = new (allocator.allocate(sizeof(Pair))) Pair;
				pair->key = key;
				pair->value = value;

				if(tail)
					tail->next = pair;
				else
					table[index] = pair;

				pair->next = 0;

				return true;
			}

			void expand()
			{
				size_t size = (this->mask + 1) << 1;
				size_t mask = size - 1;

				Pair **table = (Pair **)allocator.allocate(size * sizeof(Pair *));
				std::memset(table, 0, size * sizeof(Pair *));

				Pair **end = this->table + (this->mask + 1);

				for(Pair **slot = this->table; slot != end; ++slot)
				{
					Pair *pair = *slot;

					while(pair)
					{
						Pair *next = pair->next;

						store(table, mask, pair->key, pair->value);
						
						allocator.free(pair);

						pair = next;
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

		public:
			typedef K Key;
			typedef V Value;

			Map(size_t initial, typename Allocator::Ref::Type allocator = Allocator::Ref::standard) : allocator(allocator)
			{
				entries = 0;

				size_t size = 1 << initial;
				mask = size - 1;

				table = (Pair **)this->allocator.allocate(size * sizeof(Pair *));
				memset(table, 0, size * sizeof(Pair *));
			}

			~Map()
			{
				if(Allocator::can_free)
				{
					Pair **end = this->table + this->mask + 1;

					for(Pair **slot = this->table; slot != end; ++slot)
					{
						Pair *pair = *slot;

						while(pair)
						{
							Pair *next = pair->next;

							allocator.free(pair);

							pair = next;
						}
					}

					allocator.free(this->table);
				}
			}

			V get(K key)
			{
				size_t index = T::hash_key(key) & mask;
				Pair *pair = table[index];

				while(pair)
				{
					if(pair->key == key)
						return pair->value;
					
					pair = pair->next;
				}

				return T::invalid_value();
			}
			
			template<typename func> V try_get(K key, func fails)
			{
				size_t index = T::hash_key(key) & mask;
				Pair *pair = table[index];

				while(pair)
				{
					if(pair->key == key)
						return pair->value;
					
					pair = pair->next;
				}

				return fails();
			}
			
			template<typename func> void each_pair(func do_for_pair)
			{
				for(size_t i = 0; i <= mask; ++i)
				{
					Pair *pair = table[i];

					if(pair)
					{
						do_for_pair(pair->key, pair->value);
					}
				}
			}
			
			template<typename F> void mark_content(F mark)
			{
				for(size_t i = 0; i <= mask; ++i)
				{
					Pair *pair = table[i];
					
					if(pair)
						mark(*(void **)&table[i]);
					
					while(pair)
					{
						Pair *next =  pair->next;
						
						mark(pair->key);
						mark(pair->value);
						
						if(next)
							mark(*(void **)&pair->next);
						
						pair = next;
					}
				}
			}

			template<typename F> void mark(F mark)
			{
				mark(*(void **)&table);
				mark_content(mark);
			}

			template<typename func> V get_create(K key, func create_value)
			{
				size_t index = T::hash_key(key) & mask;
				Pair *pair = table[index];
				Pair *tail = pair;

				while(pair)
				{
					if(pair->key == key)
						return pair->value;
					
					tail = pair;
					pair = pair->next;
				}
				
				pair = new (allocator.allocate(sizeof(Pair))) Pair;
				pair->key = key;
				pair->value = create_value();

				if(tail)
					tail->next = pair;
				else
					table[index] = pair;

				pair->next = 0;

				increase();

				return pair->value;
			}
			
			V *get_ref(K key)
			{
				size_t index = T::hash_key(key) & mask;
				Pair *pair = table[index];

				while(pair)
				{
					if(pair->key == key)
						return &pair->value;
					
					pair = pair->next;
				}

				return 0;
			}

			size_t get_entries()
			{
				return entries;
			}
			
			bool has(K key)
			{
				size_t index = T::hash_key(key) & mask;
				Pair *pair = table[index];

				while(pair)
				{
					if(pair->key == key)
						return true;
					
					pair = pair->next;
				}

				return false;
			}

			void set(K key, V value)
			{
				if(store(table, mask, key, value))
					increase();
			}
			
			typename Allocator::Ref::Type get_allocator()
			{
				return allocator.get_reference();
			}
	};
};
