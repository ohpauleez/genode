/*
 * \brief  Core implementation of the PD session interface
 * \author Christian Helmuth
 * \date   2006-07-17
 *
 * FIXME arg_string and quota missing
 */

/*
 * Copyright (C) 2006-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode */
#include <base/printf.h>

/* Core */
#include <util.h>
#include <pd_session_component.h>
#include <cpu_session_component.h>

using namespace Genode;


Pd_session_component::~Pd_session_component()
{
	Lock::Guard cap_lock(_cap_lock);

	for (Cap_object *obj; (obj = _cap_list.first()); ) {
		Object_pool<Cpu_thread_component>::Guard
			cpu_thread(_thread_ep->lookup_and_lock(obj->local_name()));

		if (cpu_thread) {

			Platform_thread *p_thread = cpu_thread->platform_thread();

			_pd.unbind_thread(p_thread);
		}

		_cap_list.remove(obj);
		destroy(&_cap_slab, obj);
	}
}

int Pd_session_component::bind_thread(Thread_capability thread)
{
	Object_pool<Cpu_thread_component>::Guard cpu_thread(_thread_ep->lookup_and_lock(thread));
	if (!cpu_thread) return -1;

	if (cpu_thread->bound()) {
		PWRN("rebinding of threads not supported");
		return -2;
	}

	Platform_thread *p_thread = cpu_thread->platform_thread();

	_pd.bind_thread(p_thread);
	cpu_thread->bound(true);

	Lock::Guard cap_lock(_cap_lock);

	/* create cap object */
	Cap_object * pt_cap = new (&_cap_slab) Cap_object(thread.local_name());
	if (!pt_cap)
		return -3;

	_cap_list.insert(pt_cap);

	return 0;
}


int Pd_session_component::assign_parent(Parent_capability parent)
{
	return _pd.assign_parent(parent);
}
