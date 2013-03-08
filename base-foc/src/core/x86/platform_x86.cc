/*
 * \brief  Platform support specific to x86
 * \author Christian Helmuth
 * \author Stefan Kalkowski
 * \date   2006-04-11
 */

/*
 * Copyright (C) 2006-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/printf.h>
#include <irq_session/irq_session.h>

#include "platform.h"
#include "util.h"

/* Fiasco.OC includes */
namespace Fiasco {
#include <l4/sys/ipc.h>
#include <l4/sys/icu.h>
}

void Genode::Platform::_setup_io_port_alloc()
{
	/* setup allocator */
	_io_port_alloc.add_range(0, 0x10000);
}


void Genode::Platform::setup_irq_mode(unsigned irq_number, unsigned trigger,
                                      unsigned polarity)
{
	using namespace Fiasco;

	l4_umword_t mode;

	/* set edge-high as default for legacy interrupts */
	if (irq_number < 16
	 && trigger  == Irq_session::TRIGGER_UNCHANGED
	 && polarity == Irq_session::TRIGGER_UNCHANGED) {

		mode = L4_IRQ_F_POS_EDGE;

	} else {

		/*
		 * Translate ACPI interrupt mode (trigger/polarity) to Fiasco APIC
		 * values. Default is level low for IRQs > 15
		 */
		mode  = (trigger == Irq_session::TRIGGER_LEVEL) ||
		        (irq_number > 15 && trigger == Irq_session::TRIGGER_UNCHANGED)
		        ? L4_IRQ_F_LEVEL : L4_IRQ_F_EDGE;

		mode |= (polarity == Irq_session::POLARITY_LOW) ||
		        (irq_number > 15 && polarity == Irq_session::POLARITY_UNCHANGED)
		        ? L4_IRQ_F_NEG   : L4_IRQ_F_POS;
	}

	/*
	 * Set mode
	 */
	if (l4_error(l4_icu_set_mode(L4_BASE_ICU_CAP, irq_number, mode)))
		PERR("Setting mode for  IRQ%u failed", irq_number);
}
