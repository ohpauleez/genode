/*
 * \brief  Implementation of IRQ session component
 * \author Norman Feske
 * \date   2015-05-01
 */

/*
 * Copyright (C) 2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/printf.h>
#include <base/sleep.h>

/* core includes */
#include <irq_root.h>

namespace Genode {
       typedef Irq_proxy<Thread<1024 * sizeof(addr_t)> > Irq_proxy_base;
       class Irq_proxy_component;
}

using namespace Genode;


/**
 * Platform-specific proxy code
 */

class Genode::Irq_proxy_component : public Irq_proxy_base
{
	private:

		bool _irq_attached;

	protected:

		bool _associate() { return true; }

		void _wait_for_irq()
		{
			PDBG("not implemented");
		}

		void _ack_irq()
		{
			PDBG("not implemented");
		}

	public:

		Irq_proxy_component(long irq_number)
		:
			Irq_proxy(irq_number),
			_irq_attached(false)
		{
			_start();
		}
};


/***************************
 ** IRQ session component **
 ***************************/

void Irq_session_component::ack_irq()
{
	if (!_proxy) {
		PERR("Expected to find IRQ proxy for IRQ %02x", _irq_number);
		return;
	}

	_proxy->ack_irq();
}


Irq_session_component::Irq_session_component(Range_allocator *irq_alloc,
                                             const char      *args)
:
	_irq_alloc(irq_alloc)
{
	long irq_number = Arg_string::find_arg(args, "irq_number").long_value(-1);
	if (irq_number == -1) {
		PERR("invalid IRQ number requested");

		throw Root::Unavailable();
	}

	/* check if IRQ thread was started before */
	_proxy = Irq_proxy_component::get_irq_proxy<Irq_proxy_component>(irq_number, irq_alloc);
	if (!_proxy) {
		PERR("unavailable IRQ %lx requested", irq_number);
		throw Root::Unavailable();
	}

	_irq_number = irq_number;
}


Irq_session_component::~Irq_session_component()
{
	if (!_proxy) return;

	if (_irq_sigh.valid())
		_proxy->remove_sharer(&_irq_sigh);
}


void Irq_session_component::sigh(Genode::Signal_context_capability sigh)
{
	if (!_proxy) {
		PERR("signal handler got not registered - irq thread unavailable");
		return;
	}

	Genode::Signal_context_capability old = _irq_sigh;

	if (old.valid() && !sigh.valid())
		_proxy->remove_sharer(&_irq_sigh);

	_irq_sigh = sigh;

	if (!old.valid() && sigh.valid())
		_proxy->add_sharer(&_irq_sigh);
}
