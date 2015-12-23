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

#ifndef _INCLUDE__BASE__ENTRYPOINT_H_
#define _INCLUDE__BASE__ENTRYPOINT_H_

#include <util/volatile_object.h>
#include <base/rpc_server.h>
#include <base/signal.h>
#include <base/thread.h>

namespace Genode {
	class Startup;
	class Entrypoint;
	class Environment;
}


class Genode::Entrypoint
{
	private:

		struct Signal_proxy
		{
			GENODE_RPC(Rpc_signal, void, signal);
			GENODE_RPC_INTERFACE(Rpc_signal);
		};

		struct Signal_proxy_component :
			Rpc_object<Signal_proxy, Signal_proxy_component>
		{
			Entrypoint &ep;
			Signal_proxy_component(Entrypoint &ep) : ep(ep) { }

			void signal()
			{
				try {
					Signal sig = ep._sig_rec.pending_signal();
					ep._dispatch_signal(sig);
				} catch (Signal_receiver::Signal_not_pending) { }
			}
		};

		enum { STACK_SIZE = 1024*sizeof(long) };

		struct Signal_proxy_thread : Thread<STACK_SIZE>
		{
			Entrypoint &ep;
			Signal_proxy_thread(Entrypoint &ep)
			:
				Thread<STACK_SIZE>("signal_proxy"),
				ep(ep)
			{ }

			void entry() override { ep._process_incoming_signals(); }
		};

		Rpc_entrypoint           _rpc_ep;
		Signal_proxy_component   _signal_proxy {*this};
		Capability<Signal_proxy> _signal_proxy_cap = _rpc_ep.manage(&_signal_proxy);
		Signal_receiver          _sig_rec;

		void _dispatch_signal(Signal &sig);

		void _process_incoming_signals();

		Lazy_volatile_object<Signal_proxy_thread> _signal_proxy_thread;

		friend class Startup;


		/**
		 * Called by the startup code only
		 */
		Entrypoint(Cap_session &cap_session, Environment &env);

	public:

		Entrypoint(Cap_session &cap_session, size_t stack_size, char const *name);

		/**
		 * Associate RPC object with the entry point
		 */
		template <typename RPC_INTERFACE, typename RPC_SERVER>
		Capability<RPC_INTERFACE>
		manage(Rpc_object<RPC_INTERFACE, RPC_SERVER> &obj)
		{
			return _rpc_ep.manage(&obj);
		}

		/**
		 * Dissolve RPC object from entry point
		 */
		template <typename RPC_INTERFACE, typename RPC_SERVER>
		void dissolve(Rpc_object<RPC_INTERFACE, RPC_SERVER> &obj)
		{
			_rpc_ep.dissolve(&obj);
		}

		/**
		 * Associate signal dispatcher with entry point
		 */
		Signal_context_capability manage(Signal_dispatcher_base &);

		/**
		 * Disassociate signal dispatcher from entry point
		 */
		void dissolve(Signal_dispatcher_base &);

		/**
		 * Block and dispatch a single signal, return afterwards
		 *
		 * XXX Turn into static function that ensures that the used signal
		 *     receiver belongs to the calling entrypoint. Alternatively,
		 *     remove it.
		 */
		void wait_and_dispatch_one_signal()
		{
			Signal sig = _sig_rec.wait_for_signal();
			_dispatch_signal(sig);
		}

		/**
		 * Return RPC entrypoint
		 */
		Rpc_entrypoint &rpc_ep() { return _rpc_ep; }
};

#endif /* _INCLUDE__BASE__ENTRYPOINT_H_ */
