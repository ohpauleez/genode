/*
 * \brief  Registry of PCI devices
 * \author Norman Feske
 * \date   2015-09-09
 */

/*
 * Copyright (C) 2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _LX_EMUL__IMPL__INTERNAL__PCI_DEV_REGISTRY_H_
#define _LX_EMUL__IMPL__INTERNAL__PCI_DEV_REGISTRY_H_

///* Genode includes */
//#include <base/object_pool.h>
//#include <platform_session/connection.h>
//#include <platform_device/client.h>
//#include <io_mem_session/connection.h>
//#include <os/attached_dataspace.h>

/* Linux emulation environment includes */
#include <lx_emul/impl/internal/pci_dev.h>

namespace Lx {
	
	class Pci_dev_registry;

	/**
	 * Return singleton 'Pci_dev_registry' object
	 *
	 * Implementation must be provided by the driver.
	 */
	Pci_dev_registry *pci_dev_registry();
}

class Lx::Pci_dev_registry
{
	private:

		List<Pci_dev> _devs;

	public:

		void insert(Pci_dev *pci_dev)
		{
			PDBG("insert pci_dev %p", pci_dev);
			_devs.insert(pci_dev);
		}

		Genode::Io_mem_dataspace_capability io_mem(resource_size_t phys)
		{
			enum { PCI_ROM_RESOURCE = 6 };

			for (Pci_dev *d = _devs.first(); d; d = d->next()) {

				unsigned bar = 0;
				for (; bar < PCI_ROM_RESOURCE; bar++) {
					if ((pci_resource_flags(d, bar) & IORESOURCE_MEM) &&
					    (pci_resource_start(d, bar) == phys))
						break;
				}
				if (bar >= PCI_ROM_RESOURCE)
					continue;

				Platform::Device &device = d->client();

				unsigned resource_id =
					device.phys_bar_to_virt(bar);

				Genode::Io_mem_session_capability io_mem_cap =
					device.io_mem(resource_id);

				return Genode::Io_mem_session_client(io_mem_cap).dataspace();
			}

			PERR("Device using i/o memory of address %zx is unknown", phys);
			return Genode::Io_mem_dataspace_capability();
		}
};

#endif /* _LX_EMUL__IMPL__INTERNAL__PCI_DEV_REGISTRY_H_ */
