/*
 * Milkymist VJ SoC (Software)
 * Copyright (C) 2010 Jose Ignacio Villar
 * Copyright (C) 2010 Sebastien Bourdeauducq
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

#include <memctrl.h>
#include <board.h>
#include <stdlib.h>
#include <hw/hpdmc.h>
#include <hw/yadmc.h>

struct memctrl_desc memctrl;
int memctrl_initialized = 0;

static struct memctrl_desc memctrls[3] = {
	{
		.type = MEMCTRL_NONE,
		.name = "no memory controller",
		.reset_allowed = 0,
		.reset_function = NULL,
	
		.plltest_required = 0,
		.plltest_function = NULL,
		
		.initialization_required = 0,
		.initialization_function = NULL,
		
		.cacheflush_required = 0,
		.cacheflush_function = NULL,

		.memtest_function = NULL,
		
		.clkphase_value = 0,
		.clkphase_allowed = 0,
		.clkphase_up_function = NULL,
		.clkphase_down_function = NULL,
		.clkphase_reset_function = NULL,
		
		.idelay_value = 0,
		.idelay_allowed = 0,
		.idelay_up_function = NULL,
		.idelay_down_function = NULL,
		.idelay_reset_function = NULL,

		.dqsdelay_value = 0,
		.dqsdelay_allowed = 0,
		.dqsdelay_up_function = NULL,
		.dqsdelay_down_function = NULL,
		.dqsdelay_reset_function = NULL,
	
		.calibrationmenu_function = NULL,
		.autocalibration_function = NULL
	},
	{
		.type = DDR_SDRAM_HPDMC,
		.name = "HPDMC DDR SDRAM Controller",

		.reset_allowed = 0,
		.reset_function = hpdmc_reset,
	
		.plltest_required = 1,
		.plltest_function = hpdmc_plltest,
		
		.initialization_required = 1,
		.initialization_function = hpdmc_initialization,
		
		.cacheflush_required = 1,
		.cacheflush_function = hpdmc_cacheflush,

		.memtest_function = hpdmc_memtest,
		
		.clkphase_value = 0,
		.clkphase_allowed = 0,
		.clkphase_up_function = hpdmc_clkphase_up,
		.clkphase_down_function = hpdmc_clkphase_down,
		.clkphase_reset_function = hpdmc_clkphase_reset,
		
		.idelay_value = 0,
		.idelay_allowed = 1,
		.idelay_up_function = hpdmc_idelay_up,
		.idelay_down_function = hpdmc_idelay_down,
		.idelay_reset_function = hpdmc_idelay_reset,

		.dqsdelay_value = 5,
		.dqsdelay_allowed = 1,
		.dqsdelay_up_function = hpdmc_dqsdelay_up,
		.dqsdelay_down_function = hpdmc_dqsdelay_down,
		.dqsdelay_reset_function = hpdmc_dqsdelay_reset,
	
		.calibrationmenu_function = hpdmc_calibrationmenu,
		.autocalibration_function = hpdmc_autocalibration
	},
	{
		.type = SDR_SDRAM_YADMC,
		.name = "YADMC SDR SDRAM Controller",

		.reset_allowed = 1,
		.reset_function = yadmc_reset,
	
		.plltest_required = 1,
		.plltest_function = yadmc_plltest,
		
		.initialization_required = 1,
		.initialization_function = yadmc_initialization,
		
		.cacheflush_required = 0,
		.cacheflush_function = NULL,

		.memtest_function = yadmc_memtest,
		
		.clkphase_value = 0,
		.clkphase_allowed = 1,
		.clkphase_up_function = yadmc_clkphase_up,
		.clkphase_down_function = yadmc_clkphase_down,
		.clkphase_reset_function = yadmc_clkphase_reset,
		
		.idelay_value = 0,
		.idelay_allowed = 0,
		.idelay_up_function = NULL,
		.idelay_down_function = NULL,
		.idelay_reset_function = NULL,

		.dqsdelay_value = 3,
		.dqsdelay_allowed = 0,
		.dqsdelay_up_function = NULL,
		.dqsdelay_down_function = NULL,
		.dqsdelay_reset_function = NULL,
	
		.calibrationmenu_function = yadmc_calibrationmenu,
		.autocalibration_function = yadmc_autocalibration
	}
};

struct memctrl_desc *get_memctrl_desc_type(unsigned int type)
{
	unsigned int i;
	
	for(i=0;i<sizeof(memctrls)/sizeof(memctrls[0]);i++)
		if(memctrls[i].type == type) {
			memctrl = memctrls[i];
			return &memctrl;
		}
	return NULL;
}

struct memctrl_desc *get_memctrl_desc()
{
	if(memctrl_initialized == 0) {
		get_memctrl_desc_type(get_board_desc()->memctrl_type);
		memctrl_initialized = 1;
	}
	return &memctrl;
}

