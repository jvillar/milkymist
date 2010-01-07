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
#include <hw/hpdmc.h>
#include <memctrl.h>
#include <system.h>
#include <stdio.h>
#include <console.h>



unsigned int hpdmc_reset(int verbose) {
	if(verbose)
		printf("E: RESET not supported by HPDMC\n");
	return 1;
}

unsigned int hpdmc_plltest(int verbose) {
	int fail1, fail2;

	fail1 = !(CSR_HPDMC_IODELAY & HPDMC_PLL1_LOCKED);
	fail2 = !(CSR_HPDMC_IODELAY & HPDMC_PLL2_LOCKED);

	if(verbose) {
		printf("I: Checking if SDRAM clocking is functional:\n");
		printf("I:   PLL#1: %s\n", fail1 ? "Error" : "Locked");
		printf("I:   PLL#2: %s\n", fail2 ? "Error" : "Locked");
	}
	return(fail1 || (fail2<<1));
}


unsigned int hpdmc_initialization(int verbose) {
	volatile unsigned int i;

	/* Bring CKE high */
	CSR_HPDMC_SYSTEM = HPDMC_SYSTEM_BYPASS|HPDMC_SYSTEM_RESET|HPDMC_SYSTEM_CKE;
	for(i=0;i<2;i++);
	CSR_HPDMC_BYPASS = 0x400B;	/* Precharge All */
	for(i=0;i<2;i++);
	CSR_HPDMC_BYPASS = 0x2000F;	/* Load Extended Mode Register */
	for(i=0;i<2;i++);
	CSR_HPDMC_BYPASS = 0x123F;	/* Load Mode Register */
	for(i=0;i<200;i++);
	CSR_HPDMC_BYPASS = 0x400B;	/* Precharge All */
	for(i=0;i<2;i++);
	CSR_HPDMC_BYPASS = 0xD;		/* Auto Refresh */
	for(i=0;i<8;i++);
	CSR_HPDMC_BYPASS = 0xD;		/* Auto Refresh */
	for(i=0;i<8;i++);
	CSR_HPDMC_BYPASS = 0x23F;	/* Load Mode Register, Enable DLL */
	for(i=0;i<200;i++);
	/* Leave Bypass mode and bring up hardware controller */
	CSR_HPDMC_SYSTEM = HPDMC_SYSTEM_CKE;
	
	/* Set up pre-programmed data bus timings */
	CSR_HPDMC_IODELAY = HPDMC_IDELAY_RST;
	for(i=0;i<(get_memctrl_desc()->idelay_value);i++)
		CSR_HPDMC_IODELAY = HPDMC_IDELAY_CE|HPDMC_IDELAY_INC;
	
	for(i=0;i<get_memctrl_desc()->dqsdelay_value;i++) {
		CSR_HPDMC_IODELAY = HPDMC_DQSDELAY_CE|HPDMC_DQSDELAY_INC;
		while(!(CSR_HPDMC_IODELAY & HPDMC_DQSDELAY_RDY));
	}
	
	return 0;
}

unsigned int hpdmc_cacheflush(int verbose) {
	flush_bridge_cache();
	return 0;
}

unsigned int hpdmc_memtest(int size, int verbose)
{
	unsigned int *testbuf = (unsigned int *)SDRAM_BASE;
	unsigned int expected;
	unsigned int i;
	int error_shown = 0;
	int errors = 0;

	for(i=0;i<size;i++)
		testbuf[i] = i;

	/* NB. The Mico32 cache (Level-1) is write-through,
	 * therefore there is no order to flush the cache hierarchy.
	 */
	asm volatile( /* Invalidate Level-1 data cache */
		"wcsr DCC, r0\n"
		"nop\n"
	);
	hpdmc_cacheflush(verbose);

	for(i=0;i<size;i++) {
		expected = i;
		if(testbuf[i] != expected) {
			errors = errors + 1;
			if((verbose > 0 && !error_shown) || verbose > 1) {
				error_shown = 1;
				printf("\nE: Failed offset 0x%08x (got 0x%08x, expected 0x%08x)\n", i, testbuf[i], expected);
			}
		}
	}

	return errors;
}


unsigned int hpdmc_clkphase_up(int verbose) {
	if(verbose)
		printf("E: CLK PHASE UP not supported by HPDMC\n");
	return 1;
}


unsigned int hpdmc_clkphase_down(int verbose) {
	if(verbose)
		printf("E: CLK PHASE DOWN not supported by HPDMC\n");
	return 1;
}


unsigned int hpdmc_clkphase_reset(int verbose) {
	if(verbose)
		printf("E: CLK PHASE RESET not supported by HPDMC\n");
	return 1;
}


unsigned int hpdmc_idelay_up(int verbose) {
	unsigned int *idelay_value;
	idelay_value = &(get_memctrl_desc()->idelay_value);

	if(*idelay_value < 63) {
		*idelay_value = *idelay_value + 1;
		CSR_HPDMC_IODELAY = HPDMC_IDELAY_CE|HPDMC_IDELAY_INC;
	}
	return 0;
}


unsigned int hpdmc_idelay_down(int verbose) {
	unsigned int *idelay_value;
	idelay_value = &(get_memctrl_desc()->idelay_value);

	if(*idelay_value > 0) {
		*idelay_value = *idelay_value - 1;
		CSR_HPDMC_IODELAY = HPDMC_IDELAY_CE;
	}
	return 0;
}


unsigned int hpdmc_idelay_reset(int verbose) {
	unsigned int *idelay_value;
	idelay_value = &(get_memctrl_desc()->idelay_value);
	*idelay_value = 0;

	CSR_HPDMC_IODELAY = HPDMC_IDELAY_RST;
	return 0;
}


unsigned int hpdmc_dqsdelay_up(int verbose) {
	unsigned int *dqsdelay_value;
	dqsdelay_value = &(get_memctrl_desc()->dqsdelay_value);

	if(*dqsdelay_value < 255) {
		*dqsdelay_value = *dqsdelay_value + 1;
		CSR_HPDMC_IODELAY = HPDMC_DQSDELAY_CE|HPDMC_DQSDELAY_INC;
		while(!(CSR_HPDMC_IODELAY & HPDMC_DQSDELAY_RDY));
	}
	return 0;
}


unsigned int hpdmc_dqsdelay_down(int verbose) {
	unsigned int *dqsdelay_value;
	dqsdelay_value = &(get_memctrl_desc()->dqsdelay_value);

	if(*dqsdelay_value > 0) {
		*dqsdelay_value = *dqsdelay_value - 1;
		CSR_HPDMC_IODELAY = HPDMC_DQSDELAY_CE;
		while(!(CSR_HPDMC_IODELAY & HPDMC_DQSDELAY_RDY));
	}
	return 0;
}


unsigned int hpdmc_dqsdelay_reset(int verbose) {
	if(verbose)
		printf("E: DQS DELAY RESET not supported by HPDMC\n");
	return 1;
}


unsigned int hpdmc_calibrationmenu(int verbose) {
	int quit;
	char c[1];
	
	printf("================================\n");
	printf("DDR SDRAM calibration tool\n");
	printf("================================\n");
	printf("Memo:\n");
	printf("[> Input Delay\n");
	printf(" r = reset to 0 taps\n");
	printf(" u = add 1 tap\n");
	printf(" d = remove 1 tap\n");
	printf("[> DQS output phase\n");
	printf(" U = increase phase\n");
	printf(" D = decrease phase\n");
	printf("[> Misc\n");
	printf(" t = load image to framebuffer\n");
	printf(" q = quit\n");
	
	hpdmc_idelay_reset(verbose);
	
	quit = 0;
	while(!quit) {
		printf("Taps: %02d (78ps each) - DQS phase: %04d/255 \r", get_memctrl_desc()->clkphase_value, get_memctrl_desc()->dqsdelay_value, verbose);
		readstr(c,1);
		switch(c[0]) {
			case 'q':
				quit = 1;
				break;
			case 'r':
				hpdmc_idelay_reset(verbose);
				break;
			case 'u':
				hpdmc_clkphase_up(verbose);
				break;
			case 'd':
				hpdmc_clkphase_down(verbose);
				break;
			case 'U':
				hpdmc_dqsdelay_up(verbose);
				break;
			case 'D':
				hpdmc_dqsdelay_down(verbose);
				break;
		}
	}

	printf("\n");

	return 0;
}


unsigned int hpdmc_autocalibration(int verbose) {
	if(verbose)
		printf("E: AUTOCALIBRATION not supported by HPDMC\n");
	return 1;
}

