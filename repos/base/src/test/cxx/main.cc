/*
 * \brief  Test case for C++11-style reference initializer
 * \author Norman Feske
 * \date   2015-04-22
 */

/*
 * Copyright (C) 2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/printf.h>
#include <base/env.h>


struct Wrapped_value
{
	int const value;
	Wrapped_value() : value(1) { }
};


struct Object_with_ref
{
	Wrapped_value const &_value;
	Object_with_ref(Wrapped_value const &value);
};


Object_with_ref::Object_with_ref(Wrapped_value const &value)
:
	_value{value}
{ }


int main() {

	Wrapped_value value;

	Genode::printf("value: %d (expect 1)\n", value.value);

	Object_with_ref object_with_ref(value);

	int const v = object_with_ref._value.value;
	if (v != 1) {
		PERR("unexpected value %d (0x%x)", v, v);
		return 1;
	}

	PDBG("test completed successfully");
	return 0;
}
