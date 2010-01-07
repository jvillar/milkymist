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

#ifndef __HW_YADMC_H
#define __HW_YADMC_H

#include <hw/common.h>

#define YADMC_DCM_TAP_MIN_DELAY 20

#define YADMC_DCM_TAP_TYPICAL_DELAY 23

#define YADMC_DCM_TAP_MAX_DELAY 40

#define YADMC_PERIOD 20

#define CSR_YADMC_STATUS	MMPTR(0x80002000)

#define CSR_YADMC_YADMC_RST	MMPTR(0x80002000)

#define CSR_YADMC_CLKGEN_RST	MMPTR(0x80002004)

#define CSR_YADMC_PHASE_INC	MMPTR(0x80002008)

#define CSR_YADMC_PHASE_DEC	MMPTR(0x8000200C)

#define CSR_YADMC_PHASE_SWITCH	MMPTR(0x80002010)

#define SDRAM_BASE		(0x40000000)

unsigned int yadmc_reset(int verbose);
unsigned int yadmc_plltest(int verbose);
unsigned int yadmc_initialization(int verbose);
unsigned int yadmc_cacheflush(int verbose);
unsigned int yadmc_memtest(int size, int verbose);

unsigned int yadmc_clkphase_up(int verbose);
unsigned int yadmc_clkphase_down(int verbose);
unsigned int yadmc_clkphase_reset(int verbose);

unsigned int yadmc_idelay_up(int verbose);
unsigned int yadmc_idelay_down(int verbose);
unsigned int yadmc_idelay_reset(int verbose);

unsigned int yadmc_dqsdelay_up(int verbose);
unsigned int yadmc_dqsdelay_down(int verbose);
unsigned int yadmc_dqsdelay_reset(int verbose);

unsigned int yadmc_calibrationmenu(int verbose);
unsigned int yadmc_autocalibration(int verbose);

unsigned int ps_shift(int dir);

#endif /* __HW_YADMC_H */
