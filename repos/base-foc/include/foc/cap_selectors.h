/*
 * \brief  Static capability selector definitions
 * \author Norman Feske
 * \date   2009-10-02
 */

/*
 * Copyright (C) 2009-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__FOC__CAP_SELECTORS_H_
#define _INCLUDE__FOC__CAP_SELECTORS_H_

namespace Fiasco {
#include <l4/sys/consts.h>

	enum Cap_selectors {

		/**********************************************
		 ** Capability seclectors controlled by core **
		 **********************************************/

		TASK_CAP         = L4_BASE_TASK_CAP, /* use the same task cap selector
		                                        like L4Re for compatibility in
		                                        L4Linux */

		/*
		 * To not clash with other L4Re cap selector constants (e.g.: L4Linux)
		 * leave the following selectors (2-7) empty
		 */

		PARENT_CAP       = 0x8UL   << L4_CAP_SHIFT, /* cap to parent session */

		/*
		 * Each thread has a designated slot in the core controlled cap
		 * selector area, where its ipc gate capability (for server threads),
		 * its irq capability (for locks), and the capability to its pager
		 * gate are stored
		 */
		THREAD_AREA_BASE = 0x9UL   << L4_CAP_SHIFT, /* offset to thread area */
		THREAD_AREA_SLOT = 0x3UL   << L4_CAP_SHIFT, /* size of one thread slot */
		THREAD_GATE_CAP  = 0,                       /* offset to the ipc gate
		                                               cap selector in the slot */
		THREAD_PAGER_CAP = 0x1UL   << L4_CAP_SHIFT, /* offset to the pager
		                                               cap selector in the slot */
		THREAD_IRQ_CAP   = 0x2UL   << L4_CAP_SHIFT, /* offset to the irq cap
		                                               selector in the slot */
		MAIN_THREAD_CAP  = THREAD_AREA_BASE + THREAD_GATE_CAP, /* shortcut to the
		                                                          main thread's
		                                                          gate cap */


		/*********************************************************
		 ** Capability seclectors controlled by the task itself **
		 *********************************************************/

		USER_BASE_CAP    = 0x200UL << L4_CAP_SHIFT,
	};
}

#endif /* _INCLUDE__FOC__CAP_SELECTORS_H_ */
