/*
 * \brief  Test for running services as children using the slave API
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
#include <os/slave.h>
#include <base/printf.h>
#include <base/sleep.h>
#include <cap_session/connection.h>
#include <log_session/client.h>
#include <root/client.h>


struct Log_slave_policy : Genode::Slave_policy
{
	typedef Genode::String<100> Name;
	Name unique_name;

	/*
	 * Capability to root interface of the LOG service announced by the
	 * child. The lock gets released as soon as the slave announced the
	 * service.
	 */
	Genode::Root_capability log_root_cap;
	Genode::Lock            log_root_barrier { Genode::Lock::LOCKED };


	const char **_permitted_services() const
	{
		static const char *permitted_services[] = {
			"RM", "LOG", "SIGNAL", "CAP", 0 };

		return permitted_services;
	};

	/**
	 * Constructor
	 */
	Log_slave_policy(Name const &unique_name, char const *binary_name,
	                 Genode::Rpc_entrypoint &ep)
	:
		Genode::Slave_policy(binary_name, ep, Genode::env()->ram_session()),
		unique_name(unique_name)
	{
	}

	/**
	 * block until the slave announced the LOG service
	 *
	 * Must be called only once.
	 */
	void block_for_log_announcement()
	{
		log_root_barrier.lock();
	}

	/**
	 * Open LOG session
	 */
	Genode::Capability<Genode::Log_session> log_session()
	{
		char const *session_args = "";

		Genode::Session_capability session_cap =
			Genode::Root_client(log_root_cap).session(session_args,
			                                          Genode::Affinity());

		/*
		 * The root interface returns a untyped session capability.
		 * We return a capability casted to the specific session type.
		 */
		return Genode::static_cap_cast<Genode::Log_session>(session_cap);
	}


	/****************************
	 ** Child_policy interface **
	 ****************************/

	const char *name() const override { return unique_name.string(); }

	/*
	 * This function is called when the child announces a service
	 */
	bool announce_service(const char             *service_name,
	                      Genode::Root_capability root_cap,
	                      Genode::Allocator      *,
	                      Genode::Server         *) override
	{
		PINF("child \"%s\" announced service \"%s\"",
		     unique_name.string(), service_name);

		if (Genode::strcmp("LOG", service_name) == 0) {
			log_root_cap = root_cap;
			log_root_barrier.unlock();
		}
		return false;
	}
};


struct Log_slave
{
	/*
	 * local thread that served parent-interface requests from the child
	 */
	const Genode::size_t stack_size = 4*1024*sizeof(long);
	Genode::Rpc_entrypoint ep;

	Log_slave_policy policy;

	Genode::size_t const ram_quota = 1*1024*1024;

	Genode::Slave slave { ep, policy, ram_quota };

	/**
	 * Constructor
	 */
	Log_slave(Genode::Cap_session &cap, Log_slave_policy::Name const &name)
	:
		ep(&cap, stack_size, name.string(), true),
		policy(name, "test-slave_child", ep)
	{ }
};


int main()
{
	PLOG("--- master started ---");

	static Genode::Cap_connection cap;

	static Log_slave child_1(cap, "child_1");
	static Log_slave child_2(cap, "child_2");

	/*
	 * Wait until all children have started up their services.
	 */
	child_1.policy.block_for_log_announcement();
	child_2.policy.block_for_log_announcement();

	/*
	 * Open a session to each service
	 */
	Genode::Log_session_client log_1(child_1.policy.log_session());
	Genode::Log_session_client log_2(child_2.policy.log_session());

	/*
	 * Now, we can invoke each of the sessions
	 */
	log_1.write("a message printed by child 1");
	log_2.write("a message printed by child 2");

	Genode::sleep_forever();
}
