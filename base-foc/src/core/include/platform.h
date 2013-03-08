/*
 * \brief  Fiasco.OC platform
 * \author Christian Helmuth
 * \author Norman Feske
 * \author Stefan Kalkowski
 * \date   2007-09-10
 */

/*
 * Copyright (C) 2007-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _CORE__INCLUDE__PLATFORM_H_
#define _CORE__INCLUDE__PLATFORM_H_

/* Genode includes */
#include <base/sync_allocator.h>
#include <base/allocator_avl.h>
#include <base/pager.h>

/* Core includes */
#include <cap_id_alloc.h>
#include <platform_generic.h>
#include <platform_thread.h>
#include <platform_pd.h>
#include <multiboot.h>
#include <core_mem_alloc.h>


namespace Genode {

	class Platform : public Platform_generic
	{
		private:

			typedef Core_mem_allocator::Phys_allocator Phys_allocator;

			Platform_pd       *_core_pd;        /* core protection domain object */
			Core_mem_allocator _core_mem_alloc; /* core-accessible memory */
			Phys_allocator     _io_mem_alloc;   /* MMIO allocator */
			Phys_allocator     _io_port_alloc;  /* I/O port allocator */
			Phys_allocator     _irq_alloc;      /* IRQ allocator */
			Cap_id_allocator   _cap_id_alloc;   /* capability id allocator */
			Multiboot_info     _mb_info;        /* multiboot information */
			Rom_fs             _rom_fs;         /* ROM file system */
			Rom_module         _kip_rom;        /* ROM module for Fiasco KIP */
			addr_t             _vm_start;       /* begin of virtual memory */
			size_t             _vm_size;        /* size of virtual memory */

			/**
			 * Setup base resources
			 *
			 * - Map and provide KIP as ROM module
			 * - Initializes region allocator
			 * - Initializes multiboot info structure
			 */
			void _setup_basics();

			/**
			 * Setup RAM, IO_MEM, and region allocators
			 */
			void _setup_mem_alloc();

			/**
			 * Setup I/O port space allocator
			 */
			void _setup_io_port_alloc();

			/**
			 * Setup IRQ allocator
			 */
			void _setup_irq_alloc();

			/**
			 * Parse multi-boot information and update ROM database
			 */
			void _setup_rom();

		public:

			/**
			 * Return singleton instance of core pager object
			 */
			Pager_object *core_pager();

			/**
			 * Return start of UTCB area used by core threads
			 */
			static addr_t core_utcb_area_start();

			/**
			 * Set interrupt trigger/polarity (e.g., level or edge, high or low)
			 */
			static void setup_irq_mode(unsigned irq_number, unsigned trigger,
			                           unsigned polarity);

			/**
			 * Constructor
			 */
			Platform();

			/**
			 * Accessor for core pd object
			 */
			Platform_pd *core_pd() { return _core_pd; }


			/********************************
			 ** Generic platform interface **
			 ********************************/

			Allocator        *core_mem_alloc() { return &_core_mem_alloc; }
			Range_allocator  *ram_alloc()      { return  _core_mem_alloc.phys_alloc(); }
			Range_allocator  *io_mem_alloc()   { return &_io_mem_alloc;  }
			Range_allocator  *io_port_alloc()  { return &_io_port_alloc; }
			Range_allocator  *irq_alloc()      { return &_irq_alloc;     }
			Range_allocator  *region_alloc()   { return  _core_mem_alloc.virt_alloc(); }
			Cap_id_allocator *cap_id_alloc()   { return &_cap_id_alloc;  }
			addr_t            vm_start() const { return _vm_start;       }
			size_t            vm_size()  const { return _vm_size;        }
			Rom_fs           *rom_fs()         { return &_rom_fs;        }

			void wait_for_exit();

			bool supports_direct_unmap() const { return true; }
	};
}

#endif /* _CORE__INCLUDE__PLATFORM_H_ */
