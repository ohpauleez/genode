/*
 * \brief  Core-specific instance of the PD session interface
 * \author Christian Helmuth
 * \date   2006-07-17
 */

/*
 * Copyright (C) 2006-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _CORE__INCLUDE__PD_SESSION_COMPONENT_H_
#define _CORE__INCLUDE__PD_SESSION_COMPONENT_H_

/* Genode includes */
#include <base/rpc_server.h>
#include <pd_session/pd_session.h>
#include <util/arg_string.h>

#include <base/tslab.h>

/* core includes */
#include <platform_pd.h>

namespace Genode {

	class Pd_session_component : public Rpc_object<Pd_session>
	{
		private:

			/**
			 * Read and store the PD label
			 */
			struct Label {

				enum { MAX_LEN = 64 };
				char string[MAX_LEN];

				/**
				 * Constructor
				 */
				Label(char const *args)
				{
					Arg_string::find_arg(args, "label").string(string, sizeof(string), "");
				}

			} const _label;

			Platform_pd        _pd;
			Parent_capability  _parent;
			Rpc_entrypoint    *_thread_ep;

			struct Cap_object : Native_capability, List<Cap_object>::Element
			{
				Cap_object(addr_t cap_sel) : Native_capability(cap_sel) {}
			};

			Tslab<Cap_object, 128> _cap_slab;
			List<Cap_object>       _cap_list;
			Lock                   _cap_lock;

		public:

			Pd_session_component(Allocator *md_alloc,
			                     Rpc_entrypoint *thread_ep, const char *args)
			: _label(args), _pd(_label.string),  _thread_ep(thread_ep), _cap_slab(md_alloc) { }

			~Pd_session_component();

			/**************************/
			/** PD session interface **/
			/**************************/

			int bind_thread(Thread_capability);
			int assign_parent(Parent_capability);
	};
}

#endif /* _CORE__INCLUDE__PD_SESSION_COMPONENT_H_ */
