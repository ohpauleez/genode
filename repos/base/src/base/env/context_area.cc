/*
 * \brief  Process-local thread-context area
 * \author Norman Feske
 * \date   2010-01-19
 */

/*
 * Copyright (C) 2010-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/internal/platform_env_common.h>

using namespace Genode;

namespace Genode {
	Rm_session  *env_context_area_rm_session;
	Ram_session *env_context_area_ram_session;
}

