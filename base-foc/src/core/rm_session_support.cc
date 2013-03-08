/*
 * \brief  Fiasco-specific part of RM-session implementation
 * \author Stefan Kalkowski
 * \author Norman Feske
 * \date   2011-01-18
 */

/*
 * Copyright (C) 2011-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* core includes */
#include <rm_session_component.h>
#include <map_local.h>

using namespace Genode;

void Rm_client::unmap(addr_t core_local_base, addr_t virt_base, size_t size)
{
	Locked_ptr<Address_space> locked_address_space(_address_space);

	if (locked_address_space.is_valid())
		locked_address_space->flush(virt_base, size);
}
