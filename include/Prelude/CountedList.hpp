#pragma once
#include "List.hpp"

namespace Prelude
{
	template<class T, class E = T, ListEntry<E> E::*field = &E::entry> class CountedList:
		public List<T, E, field>
	{
	public:
		CountedList() : size(0) {}
		
		size_t size;
		
		void append(T *node)
		{
			size++;
			
			List<T, E, field>::append(node);
		}
	};
};
