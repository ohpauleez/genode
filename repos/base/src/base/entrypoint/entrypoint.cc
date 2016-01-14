/*
 * \brief  Entrypoint for serving RPC requests and dispatching signals
 * \author Norman Feske
 * \date   2015-12-17
 */

/*
 * Copyright (C) 2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/entrypoint.h>
#include <base/component.h>
#include <cap_session/connection.h>

using namespace Genode;


void Entrypoint::_dispatch_signal(Signal &sig)
{
	Signal_dispatcher_base *dispatcher = 0;
	dispatcher = dynamic_cast<Signal_dispatcher_base *>(sig.context());

	if (!dispatcher)
		return;

	dispatcher->dispatch(sig.num());
}


void Entrypoint::_process_incoming_signals()
{
	for (;;) {

		_sig_rec.block_for_signal();

		/*
		 * It might happen that we try to forward a signal to the
		 * entrypoint, while the context of that signal is already
		 * destroyed. In that case we will get an ipc error exception
		 * as result, which has to be caught.
		 */
		try {
			_signal_proxy_cap.call<Signal_proxy::Rpc_signal>();
		} catch (Genode::Ipc_error) { }
	}
}


Signal_context_capability Entrypoint::manage(Signal_dispatcher_base &dispatcher)
{
	return _sig_rec.manage(&dispatcher);
}


void Genode::Entrypoint::dissolve(Signal_dispatcher_base &dispatcher)
{
	_sig_rec.dissolve(&dispatcher);
}


/*
 * XXX move declarations to base-internal headers
 */
namespace Genode {

	void call_global_static_constructors();
	extern bool inhibit_tracing;
	void init_signal_thread();
}


namespace Genode {

	/*
	 * Hook for intercepting the call of the 'Component::construct' method.
	 * By overriding this weak function, the libc is able to create a task
	 * context for the component code. This context is scheduled by the
	 * libc in a cooperative fashion, i.e. when the component's entrypoint is
	 * activated.
	 */
	void call_component_construct(Environment &env) __attribute__((weak));
	void call_component_construct(Environment &env)
	{
		Component::construct(env);
	}
}


namespace {

	struct Constructor
	{
		GENODE_RPC(Rpc_construct, void, construct);
		GENODE_RPC_INTERFACE(Rpc_construct);
	};

	struct Constructor_component : Rpc_object<Constructor, Constructor_component>
	{
		Environment &env;
		Constructor_component(Environment &env) : env(env) { }

		void construct()
		{
			/* enable tracing support */
			Genode::inhibit_tracing = false;

			/*
			 * The signal thread must be initialized before the first signal
			 * context is allocated. The signal thread initializes the
			 * (kernel-specific) signal-delivery mechanism by instantiating
			 * a 'Signal_source_client'. On some kernels (i.e., NOVA), signal
			 * contexts can be created not before this object is constructed.
			 * Therefore, we create the signal thread prior calling the global
			 * static constructors, which may implicitly attempt to create
			 * signal contexts.
			 */
			Genode::init_signal_thread();

			Genode::call_global_static_constructors();

			Genode::call_component_construct(env);
		}
	};
}


Entrypoint::Entrypoint(Cap_session &cap_session, Environment &env)
:
	_rpc_ep(&cap_session, Component::stack_size(), Component::name())
{
	/*
	 * Invoke Component::construct function in the context of the entrypoint.
	 */
	Constructor_component constructor(env);

	Capability<Constructor> constructor_cap =
		_rpc_ep.manage(&constructor);

	constructor_cap.call<Constructor::Rpc_construct>();
	_rpc_ep.dissolve(&constructor);

	/*
	 * The calling initial thread becomes the signal proxy thread for this
	 * entrypoint
	 */
	_process_incoming_signals();
}


Entrypoint::Entrypoint(Cap_session &cap_session, size_t stack_size, char const *name)
:
	_rpc_ep(&cap_session, stack_size, name)
{
	_signal_proxy_thread.construct(*this);
}

