/*
 * \brief  Jitterentropy based random file system
 * \author Josef Soentgen
 * \date   2014-08-19
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <vfs/file_system_factory.h>

/* local includes */
#include <vfs_jitterentropy.h>


struct Jitterentropy_factory : Vfs::File_system_factory
{
	Jitterentropy_factory() { PDBG("called"); }

	Vfs::File_system *create(Genode::Xml_node node) override
	{
		PDBG("create jitterentropy file system");
		return new (Genode::env()->heap()) Jitterentropy_file_system(node);
	}
};


Vfs::File_system_factory &vfs_file_system_factory()
{
	static Jitterentropy_factory inst;
	PDBG("return inst");
	return inst;
}

