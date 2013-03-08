/*
 * \brief  Fiasco platform interface implementation
 * \author Christian Helmuth
 * \author Stefan Kalkowski
 * \author Norman Feske
 * \date   2006-04-11
 */

/*
 * Copyright (C) 2006-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/printf.h>
#include <base/allocator_avl.h>
#include <base/crt0.h>
#include <base/sleep.h>
#include <util/misc_math.h>

/* core includes */
#include <core_parent.h>
#include <platform.h>
#include <platform_thread.h>
#include <platform_pd.h>
#include <util.h>
#include <map_local.h>
#include <multiboot.h>

/* Fiasco includes */
namespace Fiasco {
#include <l4/sigma0/sigma0.h>
#include <l4/sys/ipc.h>
#include <l4/sys/kip>
#include <l4/sys/thread.h>
#include <l4/sys/types.h>
#include <l4/sys/utcb.h>
#include <l4/sys/debugger.h>
#include <l4/sys/factory.h>
#include <l4/sys/irq.h>
#include <l4/sys/scheduler.h>

static l4_kernel_info_t *kip;
}

using namespace Genode;


static const bool verbose              = true;
static const bool verbose_core_pf      = true;
static const bool verbose_region_alloc = false;


/****************************************
 ** Support for core memory management **
 ****************************************/

bool Core_mem_allocator::Mapped_mem_allocator::_map_local(addr_t   virt_addr,
                                                          addr_t   phys_addr,
                                                          unsigned size)
{
	return map_local(phys_addr, virt_addr, size >> get_page_size_log2());
}


/****************
 ** Core pager **
 ****************/

class Core_pager : public Pager_object
{
	private:

		enum { STACK_SIZE         = sizeof(addr_t)*1024,
		       STACK_NUM_ELEMENTS = STACK_SIZE / sizeof(long) };

		long _stack[STACK_NUM_ELEMENTS];

		static void _entry();

		Core_pager();

	public:

		/**
		 * Return singleton instance of core pager
		 */
		static Core_pager *core_pager();

		/**
		 * Pager_object interface
		 */
		int pager(Ipc_pager &ps) { /* never called */ return -1; }
};


void Core_pager::_entry()
{
	using namespace Fiasco;

	l4_umword_t label;
	l4_msgtag_t request_tag;

	while (true) {

		request_tag = l4_ipc_wait(l4_utcb(), &label, L4_IPC_NEVER);

		int ipc_error = l4_ipc_error(request_tag, l4_utcb());
		if (ipc_error) {
			PERR("core_pager: IPC error %d", ipc_error);
			continue;
		}

		if (request_tag.is_page_fault()) {

			/* read fault information */
			l4_umword_t pfa = l4_trunc_page(l4_utcb_mr()->mr[0]);
			l4_umword_t ip  = l4_utcb_mr()->mr[1];
			bool rw         = l4_utcb_mr()->mr[0] & 2;

			if (pfa < (l4_umword_t)L4_PAGESIZE) {

				/* NULL pointer access */
				PERR("Possible null pointer %s at %lx IP %lx",
				     rw ? "WRITE" : "READ", pfa, ip);
			} else {

				/* unexpected page fault within core */
				PERR("unexpected %s access within core at %lx IP %lx",
				     rw ? "WRITE" : "READ", pfa, ip);

				enter_kdebug("unexpected access");
			}
		}

		PWRN("core pager: dubious request");
		panic("core_pager");
		continue;
	}
}


Core_pager::Core_pager()
:
	Pager_object(0)
{
	class Fatal_error { };

	using namespace Fiasco;
	{
		l4_msgtag_t const tag = l4_factory_create_thread(L4_BASE_FACTORY_CAP,
		                                                 CORE_PAGER_THREAD_CAP);
		if (l4_msgtag_has_error(tag)) {
			PERR("cannot create core_pager thread");
			throw Fatal_error();
		}
	}

	/* reserve utcb area and associate thread with this task */
	l4_utcb_t * const utcb = ((l4_utcb_t *)((addr_t)l4_utcb() + L4_UTCB_OFFSET));
	l4_thread_control_start();
	l4_thread_control_bind(utcb, L4_BASE_TASK_CAP);
	l4_msgtag_t tag = l4_thread_control_commit(CORE_PAGER_THREAD_CAP);
	if (l4_msgtag_has_error(tag)) {
		PWRN("l4_thread_control_commit for core_pager failed");
		throw Fatal_error();
	}

	/* set human readable name in kernel debugger */
	l4_debugger_set_object_name(CORE_PAGER_THREAD_CAP, "core.pager");

	/* set priority */
	l4_sched_param_t params = l4_sched_param(Platform_thread::DEFAULT_PRIORITY);
	l4_scheduler_run_thread(L4_BASE_SCHEDULER_CAP, CORE_PAGER_THREAD_CAP, &params);

	/*
	 * Set core pager as pager to itself. This way, if the core pager
	 * faults (which should never happen), we deadlock instead of
	 * silently accessing illegal memory addresses. This condition
	 * can be easily detected in the kernel debugger using the 'lp'
	 * command and looking for the core pager sending a message to
	 * itself.
	 */
	l4_thread_control_start();
	l4_thread_control_pager(CORE_PAGER_THREAD_CAP);
	l4_thread_control_exc_handler(CORE_PAGER_THREAD_CAP);
	tag = l4_thread_control_commit(CORE_PAGER_THREAD_CAP);
	if (l4_msgtag_has_error(tag)) {
		PWRN("l4_thread_control_commit for pager thread failed!");
		enter_kdebug("setting pager for core.pager failed");
	}

	/* set initial instruction pointer and stack pointer */
	addr_t const sp = (addr_t)&_stack[STACK_NUM_ELEMENTS];
	addr_t const ip = (addr_t)_entry;
	tag = l4_thread_ex_regs(CORE_PAGER_THREAD_CAP, ip, sp, 0);
	if (l4_msgtag_has_error(tag)) {
		PWRN("l4_thread_ex_regs for core_pager failed");
		throw Fatal_error();
	}
}


Core_pager *Core_pager::core_pager()
{
	static Core_pager _core_pager;
	return &_core_pager;
}


/*******************************************
 ** Helper for populating core allocators **
 *******************************************/

struct Region
{
	addr_t start;
	addr_t end;

	Region() : start(0), end(0) { }
	Region(addr_t s, addr_t e) : start(s), end(e) { }

	/**
	 * Returns true if the specified range intersects with the region
	 */
	bool intersects(addr_t base, size_t size) const
	{
		return (((base + size) > start) && (base < end));
	}
};


/**
 * Log region
 */
static inline void print_region(Region r)
{
	printf("[%08lx,%08lx) %08lx", r.start, r.end, r.end - r.start);
}


/**
 * Add region to allocator
 */
static inline void add_region(Region r, Range_allocator *alloc)
{
	if (verbose_region_alloc) {
		printf("%p    add: ", &alloc); print_region(r); printf("\n");
	}

	/* adjust region */
	addr_t start = trunc_page(r.start);
	addr_t end   = round_page(r.end);

	alloc->add_range(start, end - start);
}


/**
 * Remove region from allocator
 */
static inline void remove_region(Region r, Range_allocator *alloc)
{
	if (verbose_region_alloc) {
		printf("%p remove: ", &alloc); print_region(r); printf("\n");
	}

	/* adjust region */
	addr_t start = trunc_page(r.start);
	addr_t end   = round_page(r.end);

	alloc->remove_range(start, end - start);
}


/*************************
 ** Core initialization **
 *************************/

void Platform::_setup_irq_alloc() { _irq_alloc.add_range(0, 0x100); }


extern long *__initial_sp; /* defined in 'crt0.s' */


void Platform::_setup_basics()
{
	using namespace Fiasco;
	using L4::Kip::Mem_desc;

	kip = (Fiasco::l4_kernel_info_t *)(*__initial_sp);

	if (kip->magic != L4_KERNEL_INFO_MAGIC)
		panic("Could not obtain kernel info page");

	if (verbose) {
		printf("\n");
		printf("KIP @ %p\n", kip);
		printf("    magic: %08zx\n", (size_t)kip->magic);
		printf("  version: %08zx\n", (size_t)kip->version);
		printf("           root "); printf(" esp: %08lx  eip: %08lx\n",
		       kip->root_esp, kip->root_eip);
	}

	/* add KIP as ROM module */
	_kip_rom = Rom_module((addr_t)kip, L4_PAGESIZE, "l4v2_kip");
	_rom_fs.insert(&_kip_rom);

	/* update multi-boot info pointer from KIP */
	void *mb_info_ptr = (void *)kip->user_ptr;
	_mb_info = Multiboot_info(mb_info_ptr);
	if (verbose) printf("MBI @ %p\n", mb_info_ptr);

	/* parse memory descriptors */
	_vm_start = 0; _vm_size  = 0;
	Mem_desc * const descriptors = Mem_desc::first(kip);

	for (unsigned i = 0; i < Mem_desc::count(kip); i++) {

		Mem_desc &desc = descriptors[i];

		addr_t const start = desc.start();
		addr_t const end   = desc.end();
		size_t const size  = end - start + 1;

		if (desc.is_virtual()) {
			/* we support only one VM region (here and also inside RM) */
			_vm_start = start;
			_vm_size  = size;
			continue;
		}

		switch (desc.type()) {
		case Mem_desc::Bootloader:
		case Mem_desc::Dedicated:
		case Mem_desc::Reserved:
		case Mem_desc::Shared:
		case Mem_desc::Undefined:
		case Mem_desc::Arch:
			break;

		case Mem_desc::Conventional:
			add_region(Region(start, end), _core_mem_alloc.phys_alloc());
			break;
		}
	}
	if (_vm_size == 0)
		panic("Virtual memory configuration not found");

	/* configure applicable address space but never use page0 */
	_vm_size  = _vm_start == 0 ? _vm_size - L4_PAGESIZE : _vm_size;
	_vm_start = _vm_start == 0 ? L4_PAGESIZE : _vm_start;
	add_region(Region(_vm_start, _vm_start + _vm_size - 1), _core_mem_alloc.virt_alloc());

	/* preserve context area in core's virtual address space */
	_core_mem_alloc.virt_alloc()->remove_range(Native_config::context_area_virtual_base(),
	                                           Native_config::context_area_virtual_size());

	/* preserve UTCB area from region allocator */
	_core_mem_alloc.virt_alloc()->remove_range((addr_t)l4_utcb(), L4_PAGESIZE);

	/* remove KIP, initial stack, and MBI area from region allocator */
	_core_mem_alloc.virt_alloc()->remove_range((addr_t)kip, L4_PAGESIZE);
	_core_mem_alloc.virt_alloc()->remove_range(trunc_page((addr_t)__initial_sp), L4_PAGESIZE);
	remove_region(Region((addr_t)mb_info_ptr,
	                     (addr_t)mb_info_ptr + _mb_info.size() - 1),
	              _core_mem_alloc.virt_alloc());

	/* leave zero page free to detect dereferenced NULL pointers */
	_core_mem_alloc.virt_alloc()->remove_range(0, L4_PAGESIZE);

	/* remove core program image memory from region allocator */
	addr_t img_start = (addr_t) &_prog_img_beg;
	addr_t img_end   = (addr_t) &_prog_img_end;
	for (addr_t addr = img_start; addr < img_end; addr += L4_PAGESIZE)
		touch_read_write((unsigned char volatile *)addr);

	remove_region(Region(img_start, img_end), _core_mem_alloc.virt_alloc());

	/* exclude regions reported by the kernel as reserved */
	for (unsigned i = 0; i < Mem_desc::count(kip); i++) {

		Mem_desc &desc = descriptors[i];

		if (desc.is_virtual())
			continue;

		if (desc.type() == Mem_desc::Conventional)
			continue;

		remove_region(Region(desc.start(), desc.end()), _core_mem_alloc.phys_alloc());
	}

	/*
	 * Populate allocator of memory-mapped I/O ranges
	 *
	 * Everything not reported by the kernel is considered as potential
	 * MMIO resource.
	 */
	_io_mem_alloc.add_range(0, ~0UL);

	/* exclude conventional RAM from memory-mapped I/O ranges */
	for (unsigned i = 0; i < Mem_desc::count(kip); i++) {
		Mem_desc &desc = descriptors[i];
		if (!desc.is_virtual() && desc.type() == Mem_desc::Conventional)
			remove_region(Region(desc.start(), desc.end()), &_io_mem_alloc);
	}

	/* allow BIOS regions to be handed out to user-level device drivers */
	for (unsigned i = 0; i < Mem_desc::count(kip); i++) {
		Mem_desc &desc = descriptors[i];
		if (desc.type() == Mem_desc::Arch)
			add_region(Region(desc.start(), desc.end()), &_io_mem_alloc);
	}

	/*
	 * Allow core to access all I/O ports
	 */
	l4_task_map(L4_BASE_TASK_CAP, L4_BASE_TASK_CAP, l4_iofpage(0, 16), 0);
}


void Platform::_setup_rom()
{
	/*
	 * We do not export any boot module loaded before FIRST_ROM.
	 */
	enum { FIRST_ROM = 2 };
	for (unsigned i = FIRST_ROM; i < _mb_info.num_modules();  i++) {
		Rom_module rom;
		if (!(rom = _mb_info.get_module(i)).valid()) continue;

		Rom_module *new_rom = new(core_mem_alloc()) Rom_module(rom);
		_rom_fs.insert(new_rom);

		if (verbose)
			printf(" mod[%d] [%p,%p) %s\n", i,
			       (void *)new_rom->addr(), ((char *)new_rom->addr()) + new_rom->size(),
			       new_rom->name());

		/* zero remainder of last ROM page */
		size_t count = L4_PAGESIZE - rom.size() % L4_PAGESIZE;
		if (count != L4_PAGESIZE)
			memset(reinterpret_cast<void *>(rom.addr() + rom.size()), 0, count);

		unmap_local(rom.addr(), round_page(rom.size()) / L4_PAGESIZE);

		/* remove ROM area from IO_MEM allocator */
		remove_region(Region(new_rom->addr(), new_rom->addr() + new_rom->size()), &_io_mem_alloc);
	}

	Rom_module *kip_rom = new(core_mem_alloc())
		Rom_module((addr_t)Fiasco::kip, L4_PAGESIZE, "kip");
	_rom_fs.insert(kip_rom);
}


Pager_object *Platform::core_pager() { return Core_pager::core_pager(); }


addr_t Platform::core_utcb_area_start()
{
	using namespace Fiasco;

	/*
	 * Initialize static 'base' the first time the function is called from
	 * the core.main thread. Any subsequent calls will return the same value.
	 *
	 * We leave the first two utcb slots for the core main thread and the
	 * core pager thread.
	 */
	static addr_t const base = (addr_t)l4_utcb() + 2*L4_UTCB_OFFSET;
	return base;
}


static void init_main_thread()
{
	using namespace Fiasco;

	unsigned const irq_cap_sel = MAIN_THREAD_CAP + THREAD_IRQ_CAP;
	/* create irq for main thread */
	l4_msgtag_t tag = l4_factory_create_irq(L4_BASE_FACTORY_CAP, irq_cap_sel);
	if (l4_msgtag_has_error(tag))
		PWRN("creating thread's irq failed");

	/* attach thread to irq */
	tag = l4_irq_attach(irq_cap_sel, 0, MAIN_THREAD_CAP);
	if (l4_msgtag_has_error(tag))
		PWRN("attaching thread's irq failed");

	l4_debugger_set_object_name(L4_BASE_THREAD_CAP, "core.main");

	/* set priority */
	l4_sched_param_t params = l4_sched_param(Platform_thread::DEFAULT_PRIORITY);
	l4_scheduler_run_thread(L4_BASE_SCHEDULER_CAP, L4_BASE_THREAD_CAP, &params);

	/* assign core pager to main thread */
	l4_thread_control_start();
	l4_thread_control_pager(CORE_PAGER_THREAD_CAP);
	l4_thread_control_exc_handler(CORE_PAGER_THREAD_CAP);
	tag = l4_thread_control_commit(L4_BASE_THREAD_CAP);
	if (l4_msgtag_has_error(tag)) {
		PWRN("l4_thread_control_commit for main thread failed!");
		enter_kdebug("setting pager for core.main failed");
	}
}


Platform::Platform()
:
	_io_mem_alloc(core_mem_alloc()),
	_io_port_alloc(core_mem_alloc()), _irq_alloc(core_mem_alloc()),
	_cap_id_alloc(core_mem_alloc())
{
	/* remember start of core's UTCB area */
	core_utcb_area_start();

	_setup_basics();
	_setup_io_port_alloc();
	_setup_irq_alloc();
	_setup_rom();

	/*
	 * Create Genode capability designated for pointing to the core pager.
	 *
	 * This capability will be assigned to core threads created via the
	 * 'Thread_base' API (see 'core/thread_start.cc').
	 */
	Core_cap_index *core_pager_cap_index =
		reinterpret_cast<Core_cap_index*>(cap_map()->insert(cap_id_alloc()->alloc(),
		                                                    Fiasco::CORE_PAGER_THREAD_CAP));
	Native_capability core_pager_cap(core_pager_cap_index);
	core_pager()->cap(core_pager_cap);

	if (verbose) {
		printf(":ram_alloc: ");    _core_mem_alloc.phys_alloc()->raw()->dump_addr_tree();
		printf(":region_alloc: "); _core_mem_alloc.virt_alloc()->raw()->dump_addr_tree();
		printf(":io_mem: ");       _io_mem_alloc.raw()->dump_addr_tree();
		printf(":io_port: ");      _io_port_alloc.raw()->dump_addr_tree();
		printf(":irq: ");          _irq_alloc.raw()->dump_addr_tree();
		printf(":rom_fs: ");       _rom_fs.print_fs();
	}

	init_main_thread();

	Core_cap_index* pd_cap_index =
		reinterpret_cast<Core_cap_index*>(cap_map()->insert(cap_id_alloc()->alloc(), Fiasco::L4_BASE_TASK_CAP));

	/* setup pd object for core pd */
	_core_pd = new(core_mem_alloc()) Platform_pd(pd_cap_index);
}


/********************************
 ** Generic platform interface **
 ********************************/

void Platform::wait_for_exit()
{
	/*
	 * On Fiasco, Core never exits. So let us sleep forever.
	 */
	sleep_forever();
}


void Core_parent::exit(int exit_value) { }

