/*
 * \brief  SDHCI controller registers
 * \author Norman Feske
 * \author Christian Helmuth
 * \date   2014-09-21
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _SDHCI_H_
#define _SDHCI_H_

/* Genode includes */
#include <util/mmio.h>
#include <os/attached_ram_dataspace.h>
#include <irq_session/connection.h>
#include <drivers/board_base.h>

/* local includes */
#include <sd_card.h>

struct Sdhci : Genode::Mmio
{
	enum { verbose = true };

	typedef Genode::size_t size_t;

	struct Resp0 : Register<0x10, 32> { };
	struct Resp1 : Register<0x14, 32> { };
	struct Resp2 : Register<0x18, 32> { };
	struct Resp3 : Register<0x1c, 32> { };

	struct Control0 : Register<0x28, 32>
	{
		struct Hctl_dwidth : Bitfield<1, 1>
		{
			enum { FOUR = 1 };
		};
	};

	struct Control1 : Register<0x2c, 32>
	{
		struct Clk_internal_en     : Bitfield<0, 1> { };
		struct Clk_internal_stable : Bitfield<1, 1> { };
		struct Clk_en              : Bitfield<2, 1> { };

		struct Clk_freq8 : Bitfield<8, 8> { };
		struct Clk_freq_ms2 : Bitfield<6, 2> { };
		struct Srst_hc   : Bitfield<24, 1> { };
//		struct Srst_cmd  : Bitfield<25, 1> { };
//		struct Srst_data : Bitfield<26, 1> { };
	};

	struct Status : Register<0x24, 32>
	{
		struct Inhibit : Bitfield<0, 2> { };
	};

	struct Arg1 : Register<0x8, 32> { };

	struct Cmdtm : Register<0xc, 32>
	{
		struct Index : Bitfield<24, 6> { };
		struct Isdata : Bitfield<21, 1> { };
		struct Tm_blkcnt_en : Bitfield<1, 1> { };
		struct Tm_multi_block : Bitfield<5, 1> { };
		struct Tm_auto_cmd_en : Bitfield<2, 2>
		{
			enum { CMD12 = 1 };
		};
		struct Tm_dat_dir : Bitfield<4, 1>
		{
			enum { WRITE = 0, READ = 1 };
		};
		struct Rsp_type : Bitfield<16, 2>
		{
			enum Response { RESPONSE_NONE             = 0,
			                RESPONSE_136_BIT          = 1,
			                RESPONSE_48_BIT           = 2,
			                RESPONSE_48_BIT_WITH_BUSY = 3 };
		};
	};

	struct Interrupt : Register<0x30, 32>
	{
		struct Cmd_done : Bitfield<0, 1> { };
		struct Data_done : Bitfield<1, 1> { };
	};

	struct Irpt_mask : Register<0x34, 32>
	{
		struct Cmd_done : Bitfield<0, 1> { };
		struct Data_done : Bitfield<1, 1> { };
	};

	struct Slotisr_ver : Register<0xfc, 32> { };

	Sdhci(Genode::addr_t const mmio_base) : Genode::Mmio(mmio_base) { }
};


struct Sdhci_controller : private Sdhci, public Sd_card::Host_controller
{
	private:

		Delayer           &_delayer;
		Sd_card::Card_info _card_info;

		Genode::Irq_connection _irq;

		Sd_card::Card_info _init()
		{
			/* reset host controller */
			write<Control1::Srst_hc>(1);

			if (!wait_for<Control1::Srst_hc>(0, _delayer)) {
				PERR("host-controller soft reset timed out");
				throw Detection_failed();
			}

			/* enable interrupt status reporting */
			write<Irpt_mask::Cmd_done>(1);
			write<Irpt_mask::Data_done>(1);

			/*
			 * We don't read the capability register as the BCM2835 always
			 * returns all bits set to zero.
			 */
			bool const supports_vdd_3_30 = true;

			PDBG("Control1: %08x", read<Control1>());
			PDBG("Slotisr_ver: %08x", read<Slotisr_ver>());

			write<Control1::Clk_freq8>(240);
			write<Control1::Clk_freq_ms2>(0);

			PDBG("Control1: %08x", read<Control1>());

//			write<Control0::Hctl_dwidth>(Control0::Hctl_dwidth::FOUR);

			write<Control1::Clk_internal_en>(1);
			if (!wait_for<Control1::Clk_internal_stable>(1, _delayer)) {
				PERR("could not set internal clock");
				throw Detection_failed();
			}

			write<Control1::Clk_en>(1);

			_delayer.usleep(10*1000);

			if (!issue_command(Sd_card::Go_idle_state())) {
				PWRN("Go_idle_state command failed");
				throw Detection_failed();
			}

			_delayer.usleep(2000);

			if (!issue_command(Sd_card::Send_if_cond())) {
				PWRN("Send_if_cond command failed");
				throw Detection_failed();
			}

			if (read<Resp0>() != 0x1aa) {
				PERR("unexpected response of Send_if_cond command");
				throw Detection_failed();
			}

			/*
			 * We need to issue the same Sd_send_op_cond command multiple
			 * times. The first time, we receive the status information. On
			 * subsequent attempts, the response tells us that the card is
			 * busy. Usually, the command is issued twice. We give up if the
			 * card is not reaching busy state after one second.
			 */

			int i = 1000;
			for (; i > 0; --i) {
				if (!issue_command(Sd_card::Sd_send_op_cond(0x18000, true))) {
					PWRN("Sd_send_op_cond command failed");
					throw Detection_failed();
				}

				if (Sd_card::Ocr::Busy::get(read<Resp0>()))
					break;

				_delayer.usleep(1000);
			}

			if (i == 0) {
				PERR("Sd_send_op_cond timed out, could no power-on SD card");
				throw Detection_failed();
			}

			PDBG("detect");
			Sd_card::Card_info card_info = _detect();
			PDBG("returned from detect");

			return card_info;
		}

		bool _wait_for_transfer_complete()
		{
			PDBG("not implemented");
			return false;
		}

	public:

		/**
		 * Constructor
		 *
		 * \param mmio_base  local base address of MMIO registers
		 */
		Sdhci_controller(Genode::addr_t const mmio_base, Delayer &delayer,
		                 unsigned irq, bool use_dma)
		:
			Sdhci(mmio_base), _delayer(delayer), _card_info(_init()), _irq(irq)
		{ }


		/****************************************
		 ** Sd_card::Host_controller interface **
		 ****************************************/

		bool _issue_command(Sd_card::Command_base const &command)
		{
			if (verbose)
				PLOG("-> index=0x%08x, arg=0x%08x, rsp_type=%d",
				     command.index, command.arg, command.rsp_type);

			if (!wait_for<Status::Inhibit>(0, _delayer)) {
				PERR("controller inhibits issueing commands");
				return false;
			}

			/* write command argument */
			write<Arg1>(command.arg);

			/* assemble command register */
			Cmdtm::access_t cmd = 0;
			Cmdtm::Index::set(cmd, command.index);
			if (command.transfer != Sd_card::TRANSFER_NONE) {

				Cmdtm::Isdata::set(cmd);
				Cmdtm::Tm_blkcnt_en::set(cmd);
				Cmdtm::Tm_multi_block::set(cmd);

				if (command.index == Sd_card::Read_multiple_block::INDEX
				 || command.index == Sd_card::Write_multiple_block::INDEX) {
					Cmdtm::Tm_auto_cmd_en::set(cmd, Cmdtm::Tm_auto_cmd_en::CMD12);

//					if (_use_dma)
//						Cmd::De::set(cmd);
				}

				/* set data-direction bit depending on the command */
				bool const read = command.transfer == Sd_card::TRANSFER_READ;
				Cmdtm::Tm_dat_dir::set(cmd, read ? Cmdtm::Tm_dat_dir::READ
				                                 : Cmdtm::Tm_dat_dir::WRITE);
			}

			Cmdtm::access_t rsp_type = 0;
			switch (command.rsp_type) {
			case Sd_card::RESPONSE_NONE:             rsp_type = Cmdtm::Rsp_type::RESPONSE_NONE;             break;
			case Sd_card::RESPONSE_136_BIT:          rsp_type = Cmdtm::Rsp_type::RESPONSE_136_BIT;          break;
			case Sd_card::RESPONSE_48_BIT:           rsp_type = Cmdtm::Rsp_type::RESPONSE_48_BIT;           break;
			case Sd_card::RESPONSE_48_BIT_WITH_BUSY: rsp_type = Cmdtm::Rsp_type::RESPONSE_48_BIT_WITH_BUSY; break;
			}
			Cmdtm::Rsp_type::set(cmd, rsp_type);

			/* write command */
			write<Cmdtm>(cmd);

			if (!wait_for<Interrupt::Cmd_done>(1, _delayer)) {
				PERR("command timed out");
				return false;
			}

			/* clear interrupt */
			write<Interrupt::Cmd_done>(1);
			write<Interrupt::Data_done>(1);

			PDBG("_issue_command returns");
			return true;
		}

		Sd_card::Card_info card_info() const
		{
			return _card_info;
		}

		Sd_card::Cid _read_cid()
		{
			Sd_card::Cid cid;
			cid.raw_0 = read<Resp0>();
			cid.raw_1 = read<Resp1>();
			cid.raw_2 = read<Resp2>();
			cid.raw_3 = read<Resp3>();
			return cid;
		}

		Sd_card::Csd _read_csd()
		{
			Sd_card::Csd csd;
			csd.csd0 = read<Resp0>();
			csd.csd1 = read<Resp1>();
			csd.csd2 = read<Resp2>();
			csd.csd3 = read<Resp3>();
			return csd;
		}

		unsigned _read_rca()
		{
//			PDBG("not implemented");
//			for (;;);
//			return 0;
			return Sd_card::Send_relative_addr::Response::Rca::get(read<Resp0>());
		}

		/**
		 * Read data blocks from SD card
		 *
		 * \return true on success
		 */
		bool read_blocks(size_t block_number, size_t block_count, char *out_buffer)
		{
			using namespace Sd_card;

//			write<Blk::Blen>(0x200);
//			write<Blk::Nblk>(block_count);
//
//			if (!issue_command(Read_multiple_block(block_number))) {
//				PERR("Read_multiple_block failed, Stat: 0x%08x", read<Stat>());
//				return false;
//			}
//
//			size_t const num_accesses = block_count*512/sizeof(Data::access_t);
//			Data::access_t *dst = (Data::access_t *)(out_buffer);
//
//			for (size_t i = 0; i < num_accesses; i++) {
//				if (!_wait_for_bre())
//					return false;
//
//				*dst++ = read<Data>();
//			}

			return _wait_for_transfer_complete();
		}

		/**
		 * Write data blocks to SD card
		 *
		 * \return true on success
		 */
		bool write_blocks(size_t block_number, size_t block_count, char const *buffer)
		{
//			using namespace Sd_card;
//
//			write<Blk::Blen>(0x200);
//			write<Blk::Nblk>(block_count);
//
//			if (!issue_command(Write_multiple_block(block_number))) {
//				PERR("Write_multiple_block failed");
//				return false;
//			}
//
//			size_t const num_accesses = block_count*512/sizeof(Data::access_t);
//			Data::access_t const *src = (Data::access_t const *)(buffer);
//
//			for (size_t i = 0; i < num_accesses; i++) {
//				if (!_wait_for_bwe())
//					return false;
//
//				write<Data>(*src++);
//			}
//
			return _wait_for_transfer_complete();
		}

		/**
		 * Read data blocks from SD card via master DMA
		 *
		 * \return true on success
		 */
		bool read_blocks_dma(size_t block_number, size_t block_count,
		                     Genode::addr_t out_buffer_phys)
		{
			return false;
		}

		/**
		 * Write data blocks to SD card via master DMA
		 *
		 * \return true on success
		 */
		bool write_blocks_dma(size_t block_number, size_t block_count,
		                      Genode::addr_t buffer_phys)
		{
			using namespace Sd_card;

			return false;
		}
};

#endif /* _SDHCI_H_ */
