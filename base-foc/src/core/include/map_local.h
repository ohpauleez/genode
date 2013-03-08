/*
 * \brief  Core-local mapping
 * \author Norman Feske
 * \author Stefan Kalkowski
 * \date   2010-02-15
 */

/*
 * Copyright (C) 2010-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _CORE__INCLUDE__MAP_LOCAL_H_
#define _CORE__INCLUDE__MAP_LOCAL_H_

/* core includes */
#include <platform.h>
#include <util.h>

/* Fiasco includes */
namespace Fiasco {
#include <l4/sys/ipc.h>
#include <l4/sigma0/sigma0.h>
#include <l4/sys/task.h>
#include <l4/sys/cache.h>
}

namespace Genode {

	/**
	 * Map pages locally within core
	 *
	 * All mapping originate from the physical address space.
	 *
	 * \param from_addr  physical source address
	 * \param to_addr    core-virtual destination address
	 * \param num_pages  number of pages to remap
	 */
	inline bool map_local(addr_t from_addr, addr_t to_addr, size_t num_pages,
	                      bool writable, bool cached)
	{
		using namespace Fiasco;

		long const perm = writable ? L4_FPAGE_RWX : L4_FPAGE_RX;
		long const type = cached ? 0 : (L4_FPAGE_BUFFERABLE << L4_FPAGE_RIGHTS_BITS);

		addr_t offset = 0;
		for (unsigned i = 0; i < num_pages; i++, offset += get_page_size()) {

			/* map from physical to core-local address space */
			l4_task_map(L4_BASE_TASK_CAP, L4_BASE_TASK_CAP,
			            l4_fpage((addr_t)from_addr + offset,
			                     get_page_size_log2(), perm),
			            (to_addr + offset) | type);
		}
		return true;
	}


	inline bool map_local(addr_t from_addr, addr_t to_addr, size_t num_pages)
	{
		return map_local(from_addr, to_addr, num_pages, true, true);
	}


	static inline bool can_use_super_page(addr_t base, size_t size)
	{
		return (base & (get_super_page_size() - 1)) == 0
		    && (size >= get_super_page_size());
	}


	static inline void unmap_local(addr_t local_base, size_t num_pages)
	{
		using namespace Fiasco;

		size_t size = num_pages << get_page_size_log2();
		addr_t addr = local_base;

		/*
		 * XXX divide operation into flexpages greater than page size
		 */
		for (; addr < local_base + size; addr += L4_PAGESIZE)
			l4_task_unmap(L4_BASE_TASK_CAP,
			              l4_fpage(addr, L4_LOG2_PAGESIZE, L4_FPAGE_RW),
			              L4_FP_ALL_SPACES);
		l4_cache_dma_coherent(local_base, local_base + size);
	}
}

#endif /* _CORE__INCLUDE__MAP_LOCAL_H_ */

