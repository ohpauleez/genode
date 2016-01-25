/*
 * \brief  Platform-specific type definitions
 * \author Norman Feske
 * \date   2009-10-02
 */

/*
 * Copyright (C) 2009-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__BASE__NATIVE_TYPES_H_
#define _INCLUDE__BASE__NATIVE_TYPES_H_

#include <base/native_config.h>
#include <base/cap_map.h>

namespace Fiasco {
#include <l4/sys/consts.h>
#include <l4/sys/types.h>
#include <l4/sys/utcb.h>
#include <l4/sys/task.h>

	struct Capability
	{
		static bool valid(l4_cap_idx_t idx) {
			return !(idx & L4_INVALID_CAP_BIT) && idx != 0; }
	};
}

namespace Genode {

	typedef Fiasco::l4_cap_idx_t Native_thread_id;


	struct Native_thread
	{
		Fiasco::l4_cap_idx_t kcap = 0;

		Native_thread() { }
		explicit Native_thread(Fiasco::l4_cap_idx_t kcap) : kcap(kcap) { }
	};


	struct Native_utcb
	{
		/*
		 * The 'Native_utcb' is located within the stack slot of the thread.
		 * We merely use it for remembering a pointer to the real UTCB, which
		 * resides somewhere in the kernel's address space.
		 */
		Fiasco::l4_utcb_t *foc_utcb = nullptr;
	};


	/**
	 * Native_capability in Fiasco.OC is just a reference to a Cap_index.
	 *
	 * As Cap_index objects cannot be copied around, but Native_capability
	 * have to, we have to use this indirection.
	 */
	class Native_capability
	{
		public:

			typedef Fiasco::l4_cap_idx_t Dst;

			struct Raw
			{
				Dst  dst;
				long local_name;
			};

		private:

			Cap_index* _idx;

		protected:

			inline void _inc()
			{
				if (_idx)
					_idx->inc();
			}

			inline void _dec()
			{
				if (_idx && !_idx->dec()) {
					cap_map()->remove(_idx);
				}
			}

		public:

			/**
			 * Default constructor creates an invalid capability
			 */
			Native_capability() : _idx(0) { }

			/**
			 * Construct capability manually
			 */
			Native_capability(Cap_index* idx)
				: _idx(idx) { _inc(); }

			Native_capability(const Native_capability &o)
			: _idx(o._idx) { _inc(); }

			~Native_capability() { _dec(); }

			/**
			 * Return Cap_index object referenced by this object
			 */
			Cap_index* idx() const { return _idx; }

			/**
			 * Overloaded comparision operator
			 */
			bool operator==(const Native_capability &o) const {
				return _idx == o._idx; }

			Native_capability& operator=(const Native_capability &o){
				if (this == &o)
					return *this;

				_dec();
				_idx = o._idx;
				_inc();
				return *this;
			}

			/*******************************************
			 **  Interface provided by all platforms  **
			 *******************************************/

			long  local_name() const { return _idx ? _idx->id() : 0;        }
			Dst   dst()        const { return _idx ? Dst(_idx->kcap()) : Dst(); }
			bool  valid()      const { return (_idx != 0) && _idx->valid(); }
	};


	typedef int Native_connection_state;

	struct Native_pd_args { };
}

#endif /* _INCLUDE__BASE__NATIVE_TYPES_H_ */
