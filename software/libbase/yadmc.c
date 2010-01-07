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
#include <hw/yadmc.h>
#include <memctrl.h>
#include <system.h>
#include <stdio.h>
#include <uart.h>
#include <console.h>

int needed_steps = (int) ((((1000*YADMC_PERIOD)/8)/YADMC_DCM_TAP_MIN_DELAY));

int max_steps = 8 * ((int) ((((1000*YADMC_PERIOD)/8)/YADMC_DCM_TAP_MIN_DELAY)));

unsigned int yadmc_reset(int verbose) {
	CSR_YADMC_YADMC_RST = 1;
	return 0;
}

unsigned int yadmc_plltest(int verbose) {
	int status;

	status = CSR_YADMC_STATUS;

	if(verbose) {
		printf("I: Checking if SDRAM clocking is functional:\n");
		printf("I:   PLLs:          %s\n", (status & 0x02) ? "Locked" : "Error");
		printf("I:   PHASE SHIFTER: %s\n", (status & 0x01) ? "Locked" : "Error");
	}
	return (status != 0x03);
}


unsigned int yadmc_initialization(int verbose) {
	int i;
	int requiered_initial_phase = get_memctrl_desc()->clkphase_value;
	int initial_dcm_phase = needed_steps;
	int steps = requiered_initial_phase - initial_dcm_phase;

	yadmc_clkphase_reset(verbose);
	if(steps > 0){
		for(i = 0; i < steps; i++) // Goes to the initial clk phase value
			yadmc_clkphase_up(verbose);
	} 
	if(steps < 0){
		for(i = 0; i > steps; i--) // Goes to the initial clk phase value
			yadmc_clkphase_down(verbose);
	} 

	return 0;
}

unsigned int yadmc_cacheflush(int verbose) {
	if(verbose)
		printf("E: CACHE FLUSH not required by YADMC\n");
	return 1;
}


unsigned int ps_shift(int dir)
{
	int times;
	
	times = 0;
	while (!(CSR_YADMC_STATUS & 0x01)) {
		if(times > 4000)
			return 1;
		times = times+1;
	}
	if(dir > 0) {
		CSR_YADMC_PHASE_INC = 1;
	}
	else {
		CSR_YADMC_PHASE_DEC = 1;
	}

	times = 0;	
	while (!(CSR_YADMC_STATUS & 0x01)) {
		if(times > 4000)
			return 1;
		times = times+1;
	}

	return 0;
}


unsigned int yadmc_memtest(int size, int verbose)
{
	volatile int *p;
	int received;
	int errors = 0;
	int error_shown = 0;
	for (p=(int *) SDRAM_BASE; p<(int *)(SDRAM_BASE+size); p++) {
		*p = (int) p;  
	}

	/* NB. The Mico32 cache (Level-1) is write-through,
	 * therefore there is no order to flush the cache hierarchy.
	 */
	asm volatile( /* Invalidate Level-1 data cache */
		"wcsr DCC, r0\n"
		"nop\n"
	);

	for (p=(int *)SDRAM_BASE; p<(int *)(SDRAM_BASE+size); p++) {
		received = *p;
		if (received != (int)p) {
			errors = errors + 1;
			if((verbose > 0 && !error_shown) || verbose > 1) {
				error_shown = 1;
				printf("\nE: Failed offset 0x%08x (got 0x%08x, expected 0x%08x)\n", (int)p, received, (int)p);
			}
		}
	}

	return errors;
}


unsigned int yadmc_clkphase_up(int verbose) {
	int i;
	unsigned int *clkphase_value;
	clkphase_value = &(get_memctrl_desc()->clkphase_value);

	if(*clkphase_value < max_steps) {

		unsigned int cur_phase_source = (*clkphase_value / (needed_steps*2));
		unsigned int next_phase_source = ((*clkphase_value+1) / (needed_steps*2));

		//unsigned int cur_phase_offset = (*clkphase_value % (needed_steps*2));
		//unsigned int next_phase_offset = ((*clkphase_value+1) % (needed_steps*2));

		if (cur_phase_source != next_phase_source) {

			CSR_YADMC_PHASE_SWITCH = 1;
			CSR_YADMC_PHASE_SWITCH = 1;
			CSR_YADMC_PHASE_SWITCH = 1; // Changes to the next clock phase driver (CLK0->CLK90 ; CLK90->CLK180 ; CLK180->CLK270 ; CLK270->CLK0)

			for(i = 0; i< (2*needed_steps); i++) { // Goes to the lowest limit for the current clock driver
				ps_shift(-1);
			}
		}
		else {
			ps_shift(1);
		}

		*clkphase_value = *clkphase_value + 1;
		CSR_YADMC_YADMC_RST = 1;
	}
	return 0;
}


unsigned int yadmc_clkphase_down(int verbose) {
	int i;
	unsigned int *clkphase_value;
	clkphase_value = &(get_memctrl_desc()->clkphase_value);

	if(*clkphase_value > 0) {

		unsigned int cur_phase_source = (*clkphase_value / (needed_steps*2));
		unsigned int next_phase_source = ((*clkphase_value-1) / (needed_steps*2));

		//unsigned int cur_phase_offset = (*clkphase_value % (needed_steps*2));
		//unsigned int next_phase_offset = ((*clkphase_value-1) % (needed_steps*2));

		if (cur_phase_source != next_phase_source) {

			CSR_YADMC_PHASE_SWITCH = 1; // Changes to the next clock phase driver (CLK0->CLK270 ; CLK90->CLK0 ; CLK180->CLK90 ; CLK270->180)

			for(i = 0; i< (2*needed_steps); i++) { // Goes to the highest limit for the current clock driver
				ps_shift(1);	
			}
		}
		else {
			ps_shift(-1);
		}

		*clkphase_value = *clkphase_value - 1;
		CSR_YADMC_YADMC_RST = 1;
	}
	return 0;
}


unsigned int yadmc_clkphase_reset(int verbose) {
	unsigned int *clkphase_value;
	clkphase_value = &(get_memctrl_desc()->clkphase_value);

	CSR_YADMC_CLKGEN_RST = 1;
	*clkphase_value = needed_steps;

	return 0;
}


unsigned int yadmc_idelay_up(int verbose) {
	if(verbose)
		printf("E: IDELAY UP not supported by YADMC\n");
	return 1;
}


unsigned int yadmc_idelay_down(int verbose) {
	if(verbose)
		printf("E: IDELAY DOWN not supported by YADMC\n");
	return 1;
}


unsigned int yadmc_idelay_reset(int verbose) {
	if(verbose)
		printf("E: IDELAY RESET not supported by YADMC\n");
	return 1;
}


unsigned int yadmc_dqsdelay_up(int verbose) {
	if(verbose)
		printf("E: DQS DELAY UP not supported by YADMC\n");
	return 1;
}


unsigned int yadmc_dqsdelay_down(int verbose) {
	if(verbose)
		printf("E: DQS DELAY DOWN not supported by YADMC\n");
	return 1;
}


unsigned int yadmc_dqsdelay_reset(int verbose) {
	if(verbose)
		printf("E: DQS DELAY RESET not supported by YADMC\n");
	return 1;
}

unsigned int phase_switch(int verbose);
unsigned int yadmc_calibrationmenu(int verbose) {
	int quit;
	int errors;
	char c;
	unsigned int cur_phase_source;
	unsigned int cur_phase_offset;
	unsigned int *clkphase_value;
	int scan_size1 = 128*1024; // 128KB
	int scan_size2 = 1024*1024; // 1MB
	int scan_size3 = 32*1024*1024; // 32MB

	clkphase_value = &(get_memctrl_desc()->clkphase_value);
	errors = 0;

	printf("================================\n");
	printf(" YADMC calibration tool\n");
	printf("================================\n");
	printf("Memo:\n");
	printf("[> CLK PHASE\n");
	printf(" r = reset to 0 taps\n");
	printf(" t = add 1 tap\n");
	printf(" b = remove 1 tap\n");
	printf("[> MEMORY TEST\n");
	printf(" a = test 128KB\n");
	printf(" A = test 128KB (full error report)\n");
	printf(" s = test 1MB\n");
	printf(" S = test 1MB (full error report)\n");
	printf(" d = test 32KB\n");
	printf(" D = test 32KB (full error report)\n");
	printf("[> Misc\n");
	printf(" P = pll test\n");
	printf(" R = reset controller\n");
	printf(" Q = quit\n");
	
	yadmc_idelay_reset(verbose);
	
	quit = 0;
	while(!quit) {
		cur_phase_source = (*clkphase_value / (needed_steps*2));
		cur_phase_offset = (*clkphase_value % (needed_steps*2));
		printf("Taps: %02d/%d (23ps each) (PHASE = (%d->%d)\r", get_memctrl_desc()->clkphase_value, max_steps, cur_phase_source, cur_phase_offset);

		c = readchar();
		switch(c) {
			case 'r':
				yadmc_clkphase_reset(verbose);
				errors = yadmc_memtest(scan_size1, 0);
				break;
			case 't':
				yadmc_clkphase_up(verbose);
				errors = yadmc_memtest(scan_size1, 0);
				break;
			case 'b':
				yadmc_clkphase_down(verbose);
				errors = yadmc_memtest(scan_size1, 0);
				break;
			case 'P':
				yadmc_plltest(verbose);
				errors = yadmc_memtest(scan_size1, 0);
				break;
			case 'R':
				yadmc_reset(verbose);
				errors = yadmc_memtest(scan_size1, 0);
				break;
			case 'a':
				errors = yadmc_memtest(scan_size1, 0);
				break;
			case 'A':
				errors = yadmc_memtest(scan_size1, 2);
				break;
			case 's':
				errors = yadmc_memtest(scan_size2, 0);
				break;
			case 'S':
				errors = yadmc_memtest(scan_size2, 2);
				break;
			case 'd':
				errors = yadmc_memtest(scan_size3, 0);
				break;
			case 'D':
				errors = yadmc_memtest(scan_size3, 2);
				break;
			case 'q':
				phase_switch(0);
				errors = yadmc_memtest(scan_size1, 0);
				break;
			case 'Q':
				quit = 1;
				break;
		}
	printf("%s ** PHASE> %d | Errors = %d\r\n", ((errors == 0) ? "OK" : "FAIL"), get_memctrl_desc()->clkphase_value, errors);
	}

	printf("\n");

	return 0;
}

unsigned int phase_switch(int verbose) {
	CSR_YADMC_PHASE_SWITCH = 1; // Changes to the next clock phase driver (CLK0->CLK90 ; CLK90->CLK180 ; CLK180->CLK270 ; CLK270->CLK0)
	return 0;
}



unsigned int yadmc_autocalibration(int verbose) {
	if(verbose)
		printf("E: AUTOCALIBRATION not supported by YADMC\n");
	return 1;
}

