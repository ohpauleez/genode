/*
 * \brief  Support functions of the RAM service
 * \author Norman Feske
 * \date   2006-07-03
 */

/*
 * Copyright (C) 2006-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* core-local includes */
#include <ram_session_component.h>
#include <map_local.h>

namespace Fiasco {
#include <l4/sys/cache.h>
}

enum { verbose_ram_ds = false };

using namespace Genode;


void Ram_session_component::_export_ram_ds(Dataspace_component *ds) { }


void Ram_session_component::_revoke_ram_ds(Dataspace_component *ds) { }


void Ram_session_component::_clear_ds(Dataspace_component *ds)
{
	/* allocate range in core's virtual address space */
	void *virt_addr;
	if (!platform()->region_alloc()->alloc(ds->size(), &virt_addr)) {
		PERR("could not allocate virtual address range in core of size %zd",
		     ds->size());
		return;
	}

	/* map the dataspace's physical pages to corresponding virtual addresses */
	size_t num_pages = ds->size() >> get_page_size_log2();
	if (!map_local(ds->phys_addr(), (addr_t)virt_addr, num_pages, true,
	               !ds->write_combined())) {
		PERR("core-local memory mapping failed");
		return;
	}

	/* clear dataspace */
	size_t num_longwords = ds->size() / sizeof(long);
	for (long *dst = (long *)virt_addr; num_longwords--;)
		*dst++ = 0;

	if (ds->write_combined())
		Fiasco::l4_cache_dma_coherent((addr_t)virt_addr,
		                              (addr_t)virt_addr + ds->size());

	/* unmap dataspace from core */
	unmap_local((addr_t)virt_addr, ds->size() >> get_page_size_log2());

	/* free core's virtual address space */
	platform()->region_alloc()->free(virt_addr, ds->size());
}

