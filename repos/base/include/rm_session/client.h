/*
 * \brief  Client-side region manager session interface
 * \author Christian Helmuth
 * \date   2006-07-11
 */

/*
 * Copyright (C) 2006-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__RM_SESSION__CLIENT_H_
#define _INCLUDE__RM_SESSION__CLIENT_H_

#include <rm_session/capability.h>
#include <base/rpc_client.h>

namespace Genode { struct Rm_session_client; }


struct Genode::Rm_session_client : Rpc_client<Rm_session>
{
	explicit Rm_session_client(Rm_session_capability session);

	Local_addr attach(Dataspace_capability ds, size_t size = 0,
	                  off_t offset = 0, bool use_local_addr = false,
	                  Local_addr local_addr = (void *)0,
	                  bool executable = false) override;

	void detach(Local_addr local_addr) override;

	Pager_capability add_client(Thread_capability thread) override;

	void remove_client(Pager_capability pager) override;

	void fault_handler(Signal_context_capability handler) override;

	State state() override;

	Dataspace_capability dataspace() override;
};

#endif /* _INCLUDE__RM_SESSION__CLIENT_H_ */
