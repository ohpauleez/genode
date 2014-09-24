/*
 * \brief  Dummy LOG service for testing the slave mechanism
 * \author Norman Feske
 * \date   2014-09-24
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/env.h>
#include <os/server.h>
#include <os/static_root.h>
#include <log_session/log_session.h>


namespace Dummy_log {

	using namespace Genode;

	struct Main;
}


struct Dummy_log::Main
{
	Server::Entrypoint &ep;

	struct Log_session_component : Rpc_object<Genode::Log_session>
	{
		/**
		 * Log_session interface
		 */
		size_t write(Genode::Log_session::String const &string) override
		{
			PLOG("-> %s <-", string.string());
			return strlen(string.string());
		}
	};

	Log_session_component log_session;

	Static_root<Genode::Log_session> log_root { ep.manage(log_session) };

	Main(Server::Entrypoint &ep) : ep(ep)
	{
		env()->parent()->announce(ep.manage(log_root));
	}
};


/************
 ** Server **
 ************/

namespace Server {

	char const *name() { return "dummy_log"; }

	size_t stack_size() { return 4*1024*sizeof(long); }

	void construct(Entrypoint &ep) { static Dummy_log::Main inst(ep); }
}
