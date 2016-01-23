/**
 * \brief  Implementations for the start of a thread
 * \author Martin Stein
 * \author Stefan Kalkowski
 * \date   2012-02-12
 */

/*
 * Copyright (C) 2012-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/thread.h>
#include <base/printf.h>
#include <base/sleep.h>
#include <base/env.h>

/* base-internal includes */
#include <base/internal/stack_allocator.h>

using namespace Genode;

namespace Genode { extern Rm_session * const env_stack_area_rm_session; }

namespace Hw {
	extern Ram_dataspace_capability _main_thread_utcb_ds;
	extern Untyped_capability       _main_thread_cap;
}

/*****************
 ** Thread_base **
 *****************/

void Thread_base::_init_platform_thread(size_t weight, Type type)
{
	if (!_cpu_session) { _cpu_session = env()->cpu_session(); }
	if (type == NORMAL) {

		/* create server object */
		char buf[48];
		name(buf, sizeof(buf));
		addr_t const utcb = (addr_t)&_stack->utcb();
		_thread_cap = _cpu_session->create_thread(weight, buf, utcb);
		return;
	}
	/* if we got reinitialized we have to get rid of the old UTCB */
	size_t const utcb_size  = sizeof(Native_utcb);
	addr_t const stack_area = Native_config::stack_area_virtual_base();
	addr_t const utcb_new   = (addr_t)&_stack->utcb() - stack_area;
	Rm_session * const rm   = env_stack_area_rm_session;
	if (type == REINITIALIZED_MAIN) { rm->detach(utcb_new); }

	/* remap initial main-thread UTCB according to stack-area spec */
	try { rm->attach_at(Hw::_main_thread_utcb_ds, utcb_new, utcb_size); }
	catch(...) {
		PERR("failed to re-map UTCB");
		while (1) ;
	}
	/* adjust initial object state in case of a main thread */
	tid().cap   = Hw::_main_thread_cap;
	_thread_cap = env()->parent()->main_thread_cap();
}


void Thread_base::_deinit_platform_thread()
{
	if (!_cpu_session)
		_cpu_session = env()->cpu_session();

	_cpu_session->kill_thread(_thread_cap);

	/* detach userland stack */
	size_t const size = sizeof(_stack->utcb());
	addr_t utcb = Stack_allocator::addr_to_base(_stack) +
	              Native_config::stack_virtual_size() - size -
	              Native_config::stack_area_virtual_base();
	env_stack_area_rm_session->detach(utcb);

	if (_pager_cap.valid()) {
		env()->rm_session()->remove_client(_pager_cap);
	}
}


void Thread_base::start()
{
	/* assign thread to protection domain */
	env()->pd_session()->bind_thread(_thread_cap);

	/* create pager object and assign it to the thread */
	_pager_cap = env()->rm_session()->add_client(_thread_cap);
	_cpu_session->set_pager(_thread_cap, _pager_cap);

	/* attach userland stack */
	try {
		Ram_dataspace_capability ds = _cpu_session->utcb(_thread_cap);
		size_t const size = sizeof(_stack->utcb());
		addr_t dst = Stack_allocator::addr_to_base(_stack) +
		             Native_config::stack_virtual_size() - size -
		             Native_config::stack_area_virtual_base();
		env_stack_area_rm_session->attach_at(ds, dst, size);
	} catch (...) {
		PERR("failed to attach userland stack");
		sleep_forever();
	}
	/* start thread with its initial IP and aligned SP */
	_cpu_session->start(_thread_cap, (addr_t)_thread_start, _stack->top());
}


void Thread_base::cancel_blocking()
{
	_cpu_session->cancel_blocking(_thread_cap);
}
