/*
 * Milkymist VJ SoC (Software)
 * Copyright (C) 2007, 2008, 2009 Sebastien Bourdeauducq
 * Copyright (C) 2010 Jose Ignacio Villar
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <hw/sysctl.h>
#include <stdlib.h>
#include <board.h>

const struct board_desc *board_desc;
int board_initialized;

static const struct board_desc boards[4] = {
	{
		.id = 0x58343031, /* X401 */
		.name = "Xilinx ML401 development board",
		.clk_frequency = 100000000,
		.fml = 1,
		.netctrl_type = NETCTRL_NONE,
		.memctrl_type = DDR_SDRAM_HPDMC,
		.memctrl_membase = 0x40000000,
		.memctrl_memsize = 64,
		.memory_card = MEMCARD_SYSTEMACE,
		.vga = 1
	},
	{
		.id = 0x53334145, /* S3AE */
		.name = "Avnet Spartan-3A evaluation kit",
		.clk_frequency = 64000000,
		.fml = 0,
		.netctrl_type = NETCTRL_NONE,
		.memctrl_type = MEMCTRL_NONE,
		.memctrl_membase = 0x00000000,
		.memctrl_memsize = 0,
		.memory_card = MEMCARD_NONE,
		.vga = 0
	},
	{
		.id = 0x5333534B, /* S3SK */
		.name = "Digilent Spartan-3E starter kit",
		.clk_frequency = 50000000,
		.fml = 0,
		.netctrl_type = OPENCORES_ETH_10_100,
		.memctrl_type = SDR_SDRAM_YADMC,
		.memctrl_membase = 0x40000000,
		.memctrl_memsize = 32,
		.memory_card = MEMCARD_NONE,
		.vga = 0

	},
	{
		.id = 0x4D4F4E45, /* MONE */
		.name = "Milkymist One",
		.clk_frequency = 80000000,
		.fml = 1,
		.netctrl_type = NETCTRL_NONE,
		.memctrl_type = DDR_SDRAM_HPDMC,
		.memctrl_membase = 0x40000000,
		.memctrl_memsize = 64,
		.memory_card = MEMCARD_MICROSD,
		.vga = 1

	}
};

const struct board_desc *get_board_desc_id(unsigned int id)
{
	unsigned int i;
	
	for(i=0;i<sizeof(boards)/sizeof(boards[0]);i++)
		if(boards[i].id == id)
			return &boards[i];
	return NULL;
}

const struct board_desc *get_board_desc()
{
	if(board_initialized != 1) {
		board_desc = get_board_desc_id(CSR_SYSTEM_ID);
		board_initialized = 1;
	}
	return board_desc;
}



