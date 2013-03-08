/*
 * \brief  Fiasco.OC-specific implementation of the IO_MEM session interface
 * \author Christian Helmuth
 * \author Stefan Kalkowski
 * \date   2006-08-28
 */

/*
 * Copyright (C) 2006-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* core includes */
#include <io_mem_session_component.h>

using namespace Genode;


void Io_mem_session_component::_unmap_local(addr_t, size_t) { }


addr_t Io_mem_session_component::_map_local(addr_t, size_t) { return 0; }
