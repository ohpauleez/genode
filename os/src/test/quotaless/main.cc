/*
 * \brief  Test for using Genode with RAM quotas disabled
 * \author Norman Feske
 * \date   2012-12-03
 *
 * This test exercises operations that would normally result in a run-time
 * error on Genode because they exceed quota limits. However, there are use
 * cases, where Genode's quota checking is undesired, in particular when using
 * Genode as middleware solution on Linux. On this platform, Genode's quotas
 * won't be enforced anyway. So configuring them seems like a nuisance. Hence,
 * there is a patch that disables quota checks throughout Genode. This program
 * tests the patch.
 */

/*
 * Copyright (C) 2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/printf.h>
#include <base/env.h>
#include <base/rpc_client.h>
#include <log_session/log_session.h>


int main(int argc, char **argv)
{
	using namespace Genode;

	printf("--- test-quotaless started ---\n");

	/*
	 * 1. Try donating too much memory to a session and thereby exhaust
	 *    own resources.
	 */
	try {
		env()->parent()->session("LOG", "ram_quota=1G");
	} catch (...) {
		PERR("could not create session with too-large quota donation");
		return -1;
	}

	PLOG("Test for exhausting quota through donation passed");

	/*
	 * 2. Deliver too little quota to a service
	 */
	try {
		Capability<Session> session = env()->parent()->session("LOG", "ram_quota=0");
		Capability<Log_session> log_session = static_cap_cast<Log_session>(session);
		log_session.call<Log_session::Rpc_write>("Using new LOG session!\n");
	} catch (...) {
		PERR("could not create session with no quota donation");
		return -2;
	}

	PLOG("Test for too-little quota donation passed");

	/*
	 * 3. Create RAM dataspace that is larger than our quota
	 */
	try {
		env()->ram_session()->alloc(1024*1024*1024);
	} catch (...) {
		PERR("could not allocate dataspace larger than our quota");
		return -3;
	}

	PLOG("Test for allocating an overly large RAM dataspace passed");

	/*
	 * 4. Create sub system via the Loader with too little memory
	 */

	/* XXX not yet implemented */

	printf("--- test-quotaless finished ---\n");
	return 0;
}
