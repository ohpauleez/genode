/*
 * \brief  Implementation of linux/io.h
 * \author Norman Feske
 * \date   2015-09-09
 */

/*
 * Copyright (C) 2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <lx_emul/impl/internal/pci_dev_registry.h>
#include <lx_emul/impl/internal/mapped_io_mem_range.h>


void *ioremap(resource_size_t phys_addr, unsigned long size)
{
	using namespace Genode;

	static Lx::List<Lx::Mapped_io_mem_range> ranges;

	/* search for the requested region within the already mapped ranges */
	for (Lx::Mapped_io_mem_range *r = ranges.first(); r; r = r->next()) {

		if (r->contains(phys_addr, size)) {
			void * const virt = (void *)(r->virt() + phys_addr - r->phys());
			PLOG("ioremap: return sub range phys 0x%lx (size %lx) to virt 0x%lx",
			     (long)phys_addr, (long)size, (long)virt);
			return virt;
		}
	}

	Io_mem_dataspace_capability ds_cap = Lx::pci_dev_registry()->io_mem(phys_addr);

	if (!ds_cap.valid()) {

		PERR("Failed to request I/O memory: [%zx,%lx)", phys_addr,
		     phys_addr + size);
		return nullptr;
	}

	Genode::size_t const ds_size = Genode::Dataspace_client(ds_cap).size();

	Lx::Mapped_io_mem_range *io_mem =
		new (env()->heap()) Lx::Mapped_io_mem_range(phys_addr, ds_size, ds_cap);

	ranges.insert(io_mem);

	PLOG("ioremap: mapped phys 0x%lx (size %lx) to virt 0x%lx",
	     (long)phys_addr, (long)size, (long)io_mem->virt());

	return (void *)io_mem->virt();
}
