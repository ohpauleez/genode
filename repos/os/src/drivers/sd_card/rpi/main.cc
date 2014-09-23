/*
 * \brief  SD-card driver for Raspberry Pi
 * \author Norman Feske
 * \date   2014-09-21
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/printf.h>
#include <os/server.h>
#include <platform_session/connection.h>

/* local includes */
#include <driver.h>


struct Main
{
	Server::Entrypoint &ep;

	Platform::Connection platform;

	struct Factory : Block::Driver_factory
	{
		Block::Driver *create() {
			return new (Genode::env()->heap()) Block::Sdhci_driver(false); }

		void destroy(Block::Driver *driver) {
			Genode::destroy(Genode::env()->heap(),
			                static_cast<Block::Sdhci_driver *>(driver)); }
	} factory;

	Block::Root root;

	Main(Server::Entrypoint &ep)
	: ep(ep), root(ep, Genode::env()->heap(), factory)
	{
		Genode::printf("--- SD card driver ---\n");

		for (int k = 0; k < 3; k++) {

		while (platform.power_state(Platform::Session::POWER_SDHCI) == 0)
			platform.power_state(Platform::Session::POWER_SDHCI, true);

//		PDBG("initial power state %d", platform.power_state(Platform::Session::POWER_SDHCI));
//		platform.power_state(Platform::Session::POWER_SDHCI, true);
//		PDBG("new power state %d", platform.power_state(Platform::Session::POWER_SDHCI));
//
//		for (unsigned i = 0; i < 16; i++)
//			PLOG("clock %d: %d", i, platform.clock_rate((Platform::Session::Clock)i));
		PLOG("emmc clock: %d", platform.clock_rate(Platform::Session::CLOCK_EMMC));
		}

		Genode::env()->parent()->announce(ep.manage(root));
	}
};


/************
 ** Server **
 ************/

namespace Server {
	char const *name()             { return "sd_card_ep";        }
	size_t stack_size()            { return 2*1024*sizeof(long); }
	void construct(Entrypoint &ep) { static Main server(ep);     }
}

