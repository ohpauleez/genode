/*
 * \brief  Fiasco.OC-specific implementation of core-local RM session
 * \author Norman Feske
 * \date   2013-03-14
 */

/*
 * Copyright (C) 2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* core includes */
#include <platform.h>
#include <core_rm_session.h>
#include <map_local.h>

using namespace Genode;


Rm_session::Local_addr
Core_rm_session::attach(Dataspace_capability ds_cap, size_t size,
                        off_t offset, bool use_local_addr,
                        Rm_session::Local_addr, bool executable)
{
	Object_pool<Dataspace_component>::Guard ds(_ds_ep->lookup_and_lock(ds_cap));
	if (!ds)
		throw Invalid_dataspace();

	if (size == 0)
		size = ds->size();

	size_t page_rounded_size = (size + get_page_size() - 1) & get_page_mask();

	if (use_local_addr) {
		PERR("Parameter 'use_local_addr' not supported within core");
		return 0;
	}

	if (offset) {
		PERR("Parameter 'offset' not supported within core");
		return 0;
	}

	/* allocate range in core's virtual address space */
	void *virt_addr;
	if (!platform()->region_alloc()->alloc(page_rounded_size, &virt_addr)) {
		PERR("Could not allocate virtual address range in core of size %zd\n",
		     page_rounded_size);
		return false;
	}

	/* map the dataspace's physical pages to corresponding virtual addresses */
	unsigned num_pages = page_rounded_size >> get_page_size_log2();
	if (!map_local(ds->phys_addr(), (addr_t)virt_addr, num_pages))
		return 0;

	_regions.insert(new (platform()->core_mem_alloc()) Entry((addr_t)virt_addr, *ds));

	return virt_addr;
}


void Core_rm_session::detach(Rm_session::Local_addr local_addr)
{
	addr_t const addr = local_addr;
	Entry *curr = _regions.first();

	/* lookup region */
	for ( ; curr; curr = curr->next())
		if (addr >= curr->core_local_addr()
		 && addr <= curr->core_local_addr() + curr->size() - 1)
			break;

	if (!curr) {
		PERR("Core_rm_session: Region lookup for address 0x%lx failed!", addr);
		return;
	}

	_regions.remove(curr);

	/* unmap dataspace from core */
	unmap_local(curr->core_local_addr(), curr->size() >> get_page_size_log2());

	/* free core's virtual address space */
	platform()->region_alloc()->free((void *)curr->core_local_addr(), curr->size());

	destroy(platform()->core_mem_alloc(), curr);
}
