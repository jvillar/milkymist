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

#ifndef __MEMCTRL_H
#define __MEMCTRL_H

#define MEMCTRL_NAME_LEN 32

struct memctrl_desc {
	unsigned int type;
	char name[MEMCTRL_NAME_LEN];

	unsigned int reset_allowed;
	unsigned int (*reset_function)(int);

	unsigned int plltest_required;
	unsigned int (*plltest_function)(int);

	unsigned int initialization_required;
	unsigned int (*initialization_function)(int);

	unsigned int cacheflush_required;
	unsigned int (*cacheflush_function)(int);

	unsigned int (*memtest_function)(int, int);

	unsigned int clkphase_value;
	unsigned int clkphase_allowed;
	unsigned int (*clkphase_up_function)(int);
	unsigned int (*clkphase_down_function)(int);
	unsigned int (*clkphase_reset_function)(int);

	unsigned int idelay_value;
	unsigned int idelay_allowed;
	unsigned int (*idelay_up_function)(int);
	unsigned int (*idelay_down_function)(int);
	unsigned int (*idelay_reset_function)(int);

	unsigned int dqsdelay_value;
	unsigned int dqsdelay_allowed;
	unsigned int (*dqsdelay_up_function)(int);
	unsigned int (*dqsdelay_down_function)(int);
	unsigned int (*dqsdelay_reset_function)(int);

	unsigned int (*calibrationmenu_function)(int);
	unsigned int (*autocalibration_function)(int);
};


struct memctrl_desc *get_memctrl_desc_type(unsigned int type);
struct memctrl_desc *get_memctrl_desc();

#endif /* __MEMCTRL_H */
