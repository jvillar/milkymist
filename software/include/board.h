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

#ifndef __BOARD_H
#define __BOARD_H

#define BOARD_NAME_LEN 32

enum {
	MEMCARD_NONE,
	MEMCARD_SYSTEMACE,
	MEMCARD_MICROSD
};

enum {
	MEMCTRL_NONE,
	DDR_SDRAM_HPDMC,
	SDR_SDRAM_YADMC,
	DDR_SDRAM_GENERIC,
	SDR_SDRAM_GENERIC
};

enum {
	NETCTRL_NONE,
	OPENCORES_ETH_10_100
};

struct board_desc {
	unsigned int id;
	char name[BOARD_NAME_LEN];
	unsigned int clk_frequency;
	unsigned int fml;
	unsigned int netctrl_type;
	unsigned int memctrl_type;
	unsigned int memctrl_membase;
	unsigned int memctrl_memsize;
	unsigned int memory_card;
	unsigned int vga;
};

extern const struct board_desc *board_desc;

const struct board_desc *get_board_desc_id(unsigned int id);
const struct board_desc *get_board_desc();

#endif /* __BOARD_H */
