#pragma once
#include "Internal/Common.hpp"

namespace Prelude
{
	template<class T> class LinkedListEntry
	{
	public:
		LinkedListEntry() : next(0), prev(0) {}

		T *next;
		T* prev;
	};

	template<class T, class E = T, LinkedListEntry<E> E::*field = &E::entry> class LinkedList
	{
	public:
		LinkedList() : first(0), last(0) {}
		
		T *first;
		T *last;
		
		bool empty()
		{
			return first == 0;
		}
		
		void remove(T *node)
		{
			prelude_debug_assert(node != 0);

			LinkedListEntry<E> &entry = node->*field;
			
			if(mirb_likely(entry.prev != 0))
				(entry.prev->*field).next = entry.next;
			else
				first = static_cast<T *>(entry.next);

			if(mirb_likely(entry.next != 0))
				(entry.next->*field).prev = entry.prev;
			else
				last = static_cast<T *>(entry.prev);
		}

		void append(T *node)
		{
			prelude_debug_assert(node != 0);

			LinkedListEntry<E> &entry = node->*field;

			entry.next = 0;

			if(prelude_likely(last != 0))
			{
				entry.prev = static_cast<E *>(last);
				(last->*field).next = static_cast<E *>(node);
				last = node;
			}
			else
			{
				first = node;
				last = node;
			}
		}

		class Iterator
		{
		private:
			T *current;

		public:
			Iterator(T *start) : current(start) {}

			void step()
			{
				current = static_cast<T *>((current->*field).next);
			}
			
			bool operator ==(const Iterator &other) const
			{
				return current == other.current;
			}
			
			bool operator !=(const Iterator &other) const
			{
				return current != other.current;
			}
			
			T &operator ++()
			{
				step();
				return *current;
			}
			
			T &operator ++(int)
			{
				T *result = current;
				step();
				return *result;
			}
			
			T *operator*() const
			{
				return current;
			}

			T &operator ()() const
			{
				return *current;
			}
		};
		
		Iterator begin()
		{
			return Iterator(first);
		}

		Iterator end()
		{
			return Iterator(0);
		}
	};
};
