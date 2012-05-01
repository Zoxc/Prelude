#pragma once
#include "Internal/Common.hpp"
#include "Allocator/Array.hpp"

namespace Prelude
{
	template<class K, class V> class MapFunctions
	{
		public:
			struct Pair
			{
				K key;
				Pair *next;
				V value;
			};
			
			static size_t hash_key(K key)
			{
				return (size_t)key;
			}

			static V invalid_value()
			{
				return 0;
			}
			
			template<typename Allocator> static Pair *allocate_pair(typename Allocator::Reference ref)
			{
				return new (Allocator(ref).allocate(sizeof(Pair))) Pair;
			}
			
			template<typename Allocator> static void free_pair(typename Allocator::Reference ref, Pair *pair)
			{
				Allocator(ref).free((void *)pair);
			}
	};

	template<class K, class V, class T = MapFunctions<K, V>, class BaseAllocator = Allocator::Standard, template<class, class> class ArrayWrapper = Allocator::Array> class Map
	{
		private:
			typedef ArrayWrapper<typename T::Pair *, BaseAllocator> Allocator;
			typedef typename T::Pair Pair;
			typedef typename Allocator::Storage Table;
			
			Table table;
			Allocator allocator;
			size_t mask;
			size_t entries;
			
			bool store(Table table, size_t mask, K key, V value)
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
				
				pair = T::template allocate_pair<BaseAllocator>(allocator.reference());
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

				Table table = allocator.allocate(size);
				
				if(!Allocator::null_references)
					std::memset(&table[0], 0, size * sizeof(Pair *));

				Pair **end = &this->table[0] + (this->mask + 1);

				for(Pair **slot = &this->table[0]; slot != end; ++slot)
				{
					Pair *pair = *slot;

					while(pair)
					{
						Pair *next = pair->next;

						store(table, mask, pair->key, pair->value);
						
						T::template free_pair<BaseAllocator>(allocator.reference(), pair);

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

			Map(size_t initial, typename Allocator::Reference allocator = Allocator::default_reference) : allocator(allocator)
			{
				entries = 0;

				size_t size = 1 << initial;
				mask = size - 1;

				table = this->allocator.allocate(size);
				
				if(!Allocator::null_references)
					std::memset(&table[0], 0, size * sizeof(Pair *));
			}

			~Map()
			{
				if(Allocator::can_free)
				{
					Pair **end = &table[0] + mask + 1;

					for(Pair **slot = &table[0]; slot != end; ++slot)
					{
						Pair *pair = *slot;

						while(pair)
						{
							Pair *next = pair->next;

							T::template free_pair<typename Allocator::Base>(allocator.reference(), pair);

							pair = next;
						}
					}

					allocator.free(table);
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
			
			template<typename func> bool each_pair(func do_for_pair)
			{
				for(size_t i = 0; i <= mask; ++i)
				{
					Pair *pair = table[i];

					if(pair)
					{
						if(!do_for_pair(pair->key, pair->value))
							return false;
					}
				}
				
				return true;
			}
			
			template<typename F> void mark_content(F mark)
			{
				for(size_t i = 0; i <= mask; ++i)
				{
					Pair *pair = table[i];
					
					if(pair)
						mark(table[i]);
				}
			}

			template<typename F> void mark(F mark)
			{
				mark(table);
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
				
				pair = T::allocate_pair(allocator.reference());
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
			
			typename Allocator::Reference get_allocator()
			{
				return allocator.reference();
			}
	};
};
