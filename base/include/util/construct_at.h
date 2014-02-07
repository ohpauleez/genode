/*
 * \brief  Utility for the manual placement of objects
 * \author Norman Feske
 * \date   2014-02-07
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__UTIL__CONSTRUCT_AT_H_
#define _INCLUDE__UTIL__CONSTRUCT_AT_H_

#include <base/stdint.h>

namespace Genode {
	template <typename T, typename... ARGS>
	static inline T *construct_at(void *, ARGS &&...);
}


/**
 * Construct object of type 'T' at location 'at', taking 'args' as constructor
 * arguments
 */
template <typename T, typename... ARGS>
static inline T *Genode::construct_at(void *at, ARGS &&... args)
{
	/**
	 * Utility to equip an existing type 'T' with a placement new operator
	 */
	struct Placeable : T
	{
		Placeable(ARGS &&... args) : T(args...) { }

		void *operator new (size_t, void *ptr) { return ptr; }
	};

	return new (at) Placeable(args...);
}

#endif /* _INCLUDE__UTIL__CONSTRUCT_AT_H_ */
