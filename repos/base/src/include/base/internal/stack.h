/*
 * \brief  Stack layout and organization
 * \author Norman Feske
 * \date   2006-04-28
 *
 * For storing thread-specific data such as the stack and thread-local data,
 * there is a dedicated portion of the virtual address space. This portion is
 * called stack area. Within this area, each thread has
 * a fixed-sized slot. The layout of each slot looks as follows
 *
 * ; lower address
 * ;   ...
 * ;   ============================ <- aligned at the slot size
 * ;
 * ;             empty
 * ;
 * ;   ----------------------------
 * ;
 * ;             stack
 * ;             (top)              <- initial stack pointer
 * ;   ---------------------------- <- address of 'Stack' object
 * ;       thread-specific data
 * ;   ----------------------------
 * ;              UTCB
 * ;   ============================ <- aligned at the slot size
 * ;   ...
 * ; higher address
 *
 * On some platforms, a user-level thread-control block (UTCB) contains
 * data shared between the user-level thread and the kernel. It is typically
 * used for transferring IPC message payload or for system-call arguments.
 * The additional stack members are a reference to the corresponding
 * 'Thread_base' object and the name of the thread.
 *
 * The stack area is a virtual memory area, initially not backed by real
 * memory. When a new thread is created, an empty slot gets assigned to the new
 * thread and populated with memory pages for the stack and thread-specific
 * data. Note that this memory is allocated from the RAM session of the
 * component environment and not accounted for when using the 'sizeof()'
 * operand on a 'Thread_base' object.
 *
 * A thread may be associated with more than one stack. Additional secondary
 * stacks can be associated with a thread, and used for user level scheduling.
 */

/*
 * Copyright (C) 2006-2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__BASE__INTERNAL__STACK_H_
#define _INCLUDE__BASE__INTERNAL__STACK_H_

/* Genode includes */
#include <base/thread.h>
#include <base/native_types.h>        /* for 'Native_utcb' */
#include <cpu/consts.h>
#include <ram_session/ram_session.h>  /* for 'Ram_dataspace_capability' type */

namespace Genode { class Stack; }


/**
 * Stack located within the stack area
 *
 * The end of a stack is placed virtual size aligned.
 */
class Genode::Stack
{
	public:

		typedef String<64> Name;

	private:

		/**
		 * Top of the stack is accessible via stack_top()
		 *
		 * Context provides the first word of the stack to prevent the
		 * overlapping of stack top and the 'stack_base' member.
		 */
		addr_t _stack[1];

		/**
		 * Thread name, used for debugging
		 */
		Name const _name;

		/**
		 * Pointer to corresponding 'Thread_base' object
		 */
		Thread_base &_thread;

		/**
		 * Virtual address of the start of the stack
		 *
		 * This address is pointing to the begin of the dataspace used for backing
		 * the stack except for the UTCB (which is managed by the kernel).
		 */
		addr_t _base = 0;

		/**
		 * Dataspace containing the backing store for the stack
		 *
		 * We keep the dataspace capability to be able to release the
		 * backing store on thread destruction.
		 */
		Ram_dataspace_capability _ds_cap;

		/*
		 * <- end of regular memory area
		 *
		 * The following part of the stack is backed by kernel-managed memory.
		 * No member variables are allowed beyond this point.
		 */

		/**
		 * Kernel-specific user-level thread control block
		 */
		Native_utcb _utcb;

	public:

		/**
		 * Constructor
		 */
		Stack(Name const &name, Thread_base &thread, addr_t base,
		      Ram_dataspace_capability ds_cap)
		:
			_name(name), _thread(thread), _base(base), _ds_cap(ds_cap)
		{ }

		/**
		 * Top of stack
		 *
		 * The alignment matches an initial stack frame, which is
		 * sufficient for the AMD64 ABI (stack top + adjustment is 16-byte
		 * aligned).
		 */
		addr_t top() const
		{
			return ((addr_t)_stack & ~0xf) - Abi::stack_adjustment();
		}

		/**
		 * Return base (the "end") of stack
		 */
		addr_t base() const { return _base; }

		/**
		 * Ensure that the stack has a given minimum size
		 *
		 * \param size  minimum stack size
		 *
		 * \throw Stack_too_large
		 * \throw Stack_alloc_failed
		 */
		void size(size_t const size);

		/**
		 * Return UTCB of the stack's thread
		 */
		Native_utcb &utcb() { return _utcb; }

		/**
		 * Return thread name
		 */
		Name name() const { return _name; }

		/**
		 * Return 'Thread_base' object of the stack's thread
		 */
		Thread_base &thread() { return _thread; }

		/**
		 * Return dataspace used as the stack's backing storage
		 */
		Ram_dataspace_capability ds_cap() const { return _ds_cap; }
};

#endif /* _INCLUDE__BASE__INTERNAL__STACK_H_ */
