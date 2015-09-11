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
	return Lx::ioremap(phys_addr, size, Genode::UNCACHED);
}


void *ioremap_wc(resource_size_t phys_addr, unsigned long size)
{
	return Lx::ioremap(phys_addr, size, Genode::WRITE_COMBINED);
}

