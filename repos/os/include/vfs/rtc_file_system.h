/*
 * \brief  Rtc file system
 * \author Josef Soentgen
 * \date   2014-08-20
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__VFS__RTC_FILE_SYSTEM_H_
#define _INCLUDE__VFS__RTC_FILE_SYSTEM_H_

/* Genode includes */
#include <rtc_session/connection.h>
#include <vfs/file_system.h>


namespace Vfs { class Rtc_file_system; }


class Vfs::Rtc_file_system : public Single_file_system
{
	private:

		Rtc::Connection _rtc;

	public:

		Rtc_file_system(Xml_node config)
		:
			Single_file_system(NODE_TYPE_CHAR_DEVICE, name(), config)
		{ }

		static char const *name() { return "rtc"; }


		/********************************
		 ** File I/O service interface **
		 ********************************/

		Write_result write(Vfs_handle *, char const *, file_size,
		                   file_size &) override
		{
			return WRITE_ERR_IO;
		}

		/**
		 * Read the current time from the RTC
		 *
		 * On each read the current time is queried and afterwards formated
		 * as '%Y-%m-%d %H:%M\n'.
		 */
		Read_result read(Vfs_handle *vfs_handle, char *dst, file_size count,
		                 file_size &out_count) override
		{
			return READ_ERR_INVALID;
		}
};

#endif /* _INCLUDE__VFS__RTC_FILE_SYSTEM_H_ */
