/*
 * \brief  Offsets within the Fiasco.OC UTCB relied on by Genode
 * \author Norman Feske
 * \date   2009-10-02
 */

/*
 * Copyright (C) 2009-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__BASE__INTERNAL__UTCB_REGS_H_
#define _INCLUDE__BASE__INTERNAL__UTCB_REGS_H_

namespace Fiasco {

	enum Utcb_regs {
		UTCB_TCR_BADGE      = 1,
		UTCB_TCR_THREAD_OBJ = 2
	};
}

#endif /* _INCLUDE__BASE__INTERNAL__UTCB_REGS_H_ */
