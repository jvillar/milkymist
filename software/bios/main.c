/*
 * Milkymist VJ SoC (Software)
 * Copyright (C) 2007, 2008, 2009 Sebastien Bourdeauducq
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


#include <stdio.h>
#include <console.h>
#include <uart.h>
#include <system.h>
#include <irq.h>
#include <cffat.h>
#include <string.h>
#include <crc.h>
#include <version.h>
#include <board.h>
#include <memctrl.h>
#include <netctrl.h>

#include <hw/hpdmc.h>
#include <hw/yadmc.h>
#include <hw/eth.h>
#include <hw/vga.h>
#include <hw/fmlbrg.h>
#include <hw/sysctl.h>
#include <hw/gpio.h>
#include <hw/uart.h>

#include <hal/time.h>

#include <net/net.h>

#include "boot.h"
#include "splash.h"


/* SDRAM functions */

static int sdram_enabled;
const struct board_desc *board_desc;
struct memctrl_desc *memctrl_desc;
struct netctrl_desc *netctrl_desc;

/* General address space functions */

#define NUMBER_OF_BYTES_ON_A_LINE 16
static void dump_bytes(unsigned int *ptr, int count, unsigned addr)
{
	char *data = (char *)ptr;
	int line_bytes = 0, i = 0;

	putsnonl("Memory dump:");
	while(count > 0){
		line_bytes =
			(count > NUMBER_OF_BYTES_ON_A_LINE)?
				NUMBER_OF_BYTES_ON_A_LINE : count;

		printf("\n0x%08x  ", addr);
		for(i=0;i<line_bytes;i++)
			printf("%02x ", *(unsigned char *)(data+i));
	
		for(;i<NUMBER_OF_BYTES_ON_A_LINE;i++)
			printf("   ");
	
		printf(" ");

		for(i=0;i<line_bytes;i++) {
			if((*(data+i) < 0x20) || (*(data+i) > 0x7e))
				printf(".");
			else
				printf("%c", *(data+i));
		}	

		for(;i<NUMBER_OF_BYTES_ON_A_LINE;i++)
			printf(" ");

		data += (char)line_bytes;
		count -= line_bytes;
		addr += line_bytes;
	}
	printf("\n");
}


static void mr(char *startaddr, char *len)
{
	char *c;
	unsigned int *addr;
	unsigned int length;

	if(*startaddr == 0) {
		printf("mr <address> [length]\n");
		return;
	}
	addr = (unsigned *)strtoul(startaddr, &c, 0);
	if(*c != 0) {
		printf("incorrect address\n");
		return;
	}
	if(*len == 0) {
		length = 1;
	} else {
		length = strtoul(len, &c, 0);
		if(*c != 0) {
			printf("incorrect length\n");
			return;
		}
	}

	dump_bytes(addr, length, (unsigned)addr);
}

static void mw(char *addr, char *value, char *count)
{
	char *c;
	unsigned int *addr2;
	unsigned int value2;
	unsigned int count2;
	unsigned int i;

	if((*addr == 0) || (*value == 0)) {
		printf("mw <address> <value> [count]\n");
		return;
	}
	addr2 = (unsigned int *)strtoul(addr, &c, 0);
	if(*c != 0) {
		printf("incorrect address\n");
		return;
	}
	value2 = strtoul(value, &c, 0);
	if(*c != 0) {
		printf("incorrect value\n");
		return;
	}
	if(*count == 0) {
		count2 = 1;
	} else {
		count2 = strtoul(count, &c, 0);
		if(*c != 0) {
			printf("incorrect count\n");
			return;
		}
	}
	for (i=0;i<count2;i++) *addr2++ = value2;
}

static void mc(char *dstaddr, char *srcaddr, char *count)
{
	char *c;
	unsigned int *dstaddr2;
	unsigned int *srcaddr2;
	unsigned int count2;
	unsigned int i;

	if((*dstaddr == 0) || (*srcaddr == 0)) {
		printf("mc <dst> <src> [count]\n");
		return;
	}
	dstaddr2 = (unsigned int *)strtoul(dstaddr, &c, 0);
	if(*c != 0) {
		printf("incorrect destination address\n");
		return;
	}
	srcaddr2 = (unsigned int *)strtoul(srcaddr, &c, 0);
	if(*c != 0) {
		printf("incorrect source address\n");
		return;
	}
	if(*count == 0) {
		count2 = 1;
	} else {
		count2 = strtoul(count, &c, 0);
		if(*c != 0) {
			printf("incorrect count\n");
			return;
		}
	}
	for (i=0;i<count2;i++) *dstaddr2++ = *srcaddr2++;
}

static void crc(char *startaddr, char *len)
{
	char *c;
	char *addr;
	unsigned int length;

	if((*startaddr == 0)||(*len == 0)) {
		printf("crc <address> <length>\n");
		return;
	}
	addr = (char *)strtoul(startaddr, &c, 0);
	if(*c != 0) {
		printf("incorrect address\n");
		return;
	}
	length = strtoul(len, &c, 0);
	if(*c != 0) {
		printf("incorrect length\n");
		return;
	}

	printf("CRC32: %08x\n", crc32((unsigned char *)addr, length));
}

/* CF filesystem functions */

static int lscb(const char *filename, const char *longname, void *param)
{
	printf("%12s [%s]\n", filename, longname);
	return 1;
}

static void ls()
{
	if(board_desc->memory_card == MEMCARD_NONE) {
		printf("E: No memory card on this board\n");
		return;
	}
	cffat_init();
	cffat_list_files(lscb, NULL);
	cffat_done();
}

static void load(char *filename, char *addr)
{
	char *c;
	unsigned int *addr2;

	if(board_desc->memory_card == MEMCARD_NONE) {
		printf("E: No memory card on this board\n");
		return;
	}

	if((*filename == 0) || (*addr == 0)) {
		printf("load <filename> <address>\n");
		return;
	}
	addr2 = (unsigned *)strtoul(addr, &c, 0);
	if(*c != 0) {
		printf("incorrect address\n");
		return;
	}
	cffat_init();
	cffat_load(filename, (char *)addr2, 16*1024*1024, NULL);
	cffat_done();
}

static void uptime()
{
	struct timestamp now;
	int days = 0;
	int hours = 0;
	int mins = 0;
	int secs = 0;

	time_get(&now);
	secs = now.sec;
	
	days = (now.sec / 86400);
	secs = (now.sec % 86400);
	hours= (secs / 3600);
	secs = (secs % 3600);
	mins = (secs / 60);
	secs = (secs % 60);

	printf("System up for %d days, %02d:%02d:%02d\n", days, hours, mins, secs);
}

static void tftp(char *filename, char *server)
{
	unsigned int ip = 0xC0A80001;

	if((*filename == 0)) {
		printf("\n\ntftp <filename> [<server ip>]\n");
		printf("\n\nserver ip defaults to 192.168.0.1\n");
	}
	else {
		if((*server != 0)) {
			ip = inet_aton(server);
		}

		netctrl_desc->srv_ip = ip;
		TftpSetFileName(filename);

		NetLoop(TFTP);
	}

	return 0;
}

static void netcfg(char *ip, char *mask, char *gateway)
{
	if((*ip == 0) || (*mask == 0) || (*gateway == 0)) {
		printf("IP:      %s", inet_ntoa(netctrl_desc->ip));
		printf("\nnetmask: %s", inet_ntoa(netctrl_desc->mask)); 
		printf("\ngateway: %s", inet_ntoa(netctrl_desc->gw_ip)); 
		printf("\n\nnetcfg [<ip> <mask> <gateway>]\n");
	}
	else {
		netctrl_desc->ip = inet_aton(ip);
		netctrl_desc->mask = inet_aton(mask);
		netctrl_desc->gw_ip = inet_aton(gateway);
		printf("Restarting network with new parameters...");
		NetStartAgain();
	}

	return 0;
}


/* Init + command line */
static void help()
{
	puts("This is the Milkymist BIOS debug shell.");
	puts("It is used for system development and maintainance, and not");
	puts("for regular operation.\n");
	puts("Available commands:");
	puts("uptime    - tell how long the system has been running");
	if(sdram_enabled && memctrl_desc->plltest_function != NULL) 
		puts("plltest    - print status of the SDRAM clocking PLLs");
	if(sdram_enabled) 
		puts("memtest    - extended SDRAM test");
	if(sdram_enabled && memctrl_desc->calibrationmenu_function != NULL)
		puts("calibrate  - run DDR SDRAM calibration tool");
	if(sdram_enabled && memctrl_desc->cacheflush_function != NULL)
		puts("flush      - flush FML bridge cache");
	puts("mr         - read address space");
	puts("mw         - write address space");
	puts("mc         - copy address space");
	puts("crc        - compute CRC32 of a part of the address space");
	if(board_desc->memory_card != MEMCARD_NONE) {
		puts("ls         - list files on the memory card");
		puts("load       - load a file from the memory card");
		puts("cardboot   - attempt booting from memory card");
	}
	puts("serialboot - attempt SFL boot");
	if(netctrl_desc->type != NETCTRL_NONE) {
		puts("netcfg     - configure self IP, netmask and gateway");
		puts("tftp       - load a file from the network using tftp");
	}

}

static char *get_token(char **str)
{
	char *c, *d;

	c = (char *)strchr(*str, ' ');
	if(c == NULL) {
		d = *str;
		*str = *str+strlen(*str);
		return d;
	}
	*c = 0;
	d = *str;
	*str = c+1;
	return d;
}


void time_tick()
{
	;
}

	
static void do_command(char *c)
{
	char *token;
	int errors;
	token = get_token(&c);

	if(strcmp(token, "uptime") == 0) uptime();

	else if(strcmp(token, "plltest") == 0) {
		if(sdram_enabled && memctrl_desc->plltest_function != NULL) 
			memctrl_desc->plltest_function(1);
		else
			printf("E: Memory controller is disabled or command is not available\n");
	}
	else if(strcmp(token, "calibrate") == 0) {
		if(sdram_enabled && memctrl_desc->calibrationmenu_function != NULL)
			memctrl_desc->calibrationmenu_function(1);
		else
			printf("E: Memory controller is disabled or command is not available\n");
	}
	else if(strcmp(token, "flush") == 0) {
		if(sdram_enabled && memctrl_desc->cacheflush_function != NULL)
			memctrl_desc->cacheflush_function(1);
		else
			printf("E: Memory controller is disabled or command is not available\n");
	}
	else if(strcmp(token, "memtest") == 0) {
		errors = memctrl_desc->memtest_function(board_desc->memctrl_memsize*1024*1024, 0);
		if(errors == 0) printf("I: MEMTEST OK\n");
		else printf("E: MEMTEST FAILED with %d errors\n", errors);
	}
	else if(strcmp(token, "mr") == 0) mr(get_token(&c), get_token(&c));
	else if(strcmp(token, "mw") == 0) mw(get_token(&c), get_token(&c), get_token(&c));
	else if(strcmp(token, "mc") == 0) mc(get_token(&c), get_token(&c), get_token(&c));
	else if(strcmp(token, "crc") == 0) crc(get_token(&c), get_token(&c));
	
	else if(strcmp(token, "ls") == 0) ls();
	else if(strcmp(token, "load") == 0) load(get_token(&c), get_token(&c));

	else if(strcmp(token, "serialboot") == 0) serialboot();
	else if(strcmp(token, "cardboot") == 0) {
		if(board_desc->memory_card != MEMCARD_NONE)
			cardboot(0);
		else
			printf("E: Data storage card is not present\n");
	}
	else if(strcmp(token, "netcfg") == 0)
	{
		if(netctrl_desc->type != NETCTRL_NONE)
			netcfg(get_token(&c), get_token(&c), get_token(&c));
		else
			printf("E: Network controller is not present\n");
	}
	else if(strcmp(token, "tftp") == 0) {
		if(netctrl_desc->type != NETCTRL_NONE) {
			tftp(get_token(&c), get_token(&c));
		}
		else
			printf("E: Network controller is not present\n");
	}

	else if(strcmp(token, "help") == 0) help();
	
	else if(strcmp(token, "") != 0)
		printf("Command not found\n");
}

static int test_user_abort()
{
	unsigned int i;
	char c;
	
	puts("I: Press Q to abort boot");
	for(i=0;i<4000000;i++) {
		if(readchar_nonblock()) {
			c = readchar();
			if(c == 'Q') {
				puts("I: Aborted boot on user request");
				return 0;
			}
		}
	}
	return 1;
}

extern unsigned int _edata;

static void crcbios()
{
	unsigned int length;
	unsigned int expected_crc;
	unsigned int actual_crc;
	
	/*
	 * _edata is located right after the end of the flat
	 * binary image. The CRC tool writes the 32-bit CRC here.
	 * We also use the address of _edata to know the length
	 * of our code.
	 */
	expected_crc = _edata;
	length = (unsigned int)&_edata;
	actual_crc = crc32((unsigned char *)0, length);
	if(expected_crc == actual_crc)
		printf("I: BIOS CRC passed (%08x)\n", actual_crc);
	else {
		printf("W: BIOS CRC failed (expected %08x, got %08x)\n", expected_crc, actual_crc);
		printf("W: The system will continue, but expect problems.\n");
	}
}

static void display_board()
{
	if(board_desc == NULL) {
		printf("E: Running on unknown board (ID=0x%08x), startup aborted.\n", CSR_SYSTEM_ID);
		while(1);
	}
	else if(memctrl_desc == NULL) {
		printf("E: Running with unknown memory controller (TYPE=0x%08x), startup aborted.\n", board_desc->memctrl_type);
		while(1);
	}
	printf("I: Running on %s using %s\n", board_desc->name, memctrl_desc->name);
}

static const char banner[] =
	"\nMILKYMIST(tm) v"VERSION" BIOS\thttp://www.milkymist.org\n"
	"(c) 2007, 2008, 2009 Sebastien Bourdeauducq\n\n"
	"This program is free software: you can redistribute it and/or modify\n"
	"it under the terms of the GNU General Public License as published by\n"
	"the Free Software Foundation, version 3 of the License.\n\n";

static void boot_sequence()
{
	if(board_desc->vga) {
		splash_display((void *)(SDRAM_BASE+1024*1024*(board_desc->memctrl_memsize-4)));
	}

	if(test_user_abort()) {
		serialboot(1);
		if(board_desc->memory_card != MEMCARD_NONE) {
			if(CSR_GPIO_IN & GPIO_DIP1)
				cardboot(1);
			else
				cardboot(0);
		}
		printf("E: No boot medium found\n");
	}
}

int main(int warm_boot, char **dummy)
{
	char buffer[64];

	irq_setmask(0);
	irq_enable(1);

	board_desc = get_board_desc();
	memctrl_desc = get_memctrl_desc();
	netctrl_desc = get_netctrl_desc();

	/* Check for double baud rate */
	if(board_desc != NULL) {
		if(CSR_GPIO_IN & GPIO_DIP2)
			CSR_UART_DIVISOR = board_desc->clk_frequency/230400/16;
	}

	/* Display a banner as soon as possible to show that the system is alive */
	putsnonl(banner);

	crcbios();
	display_board();

	time_init();

	sdram_enabled = 1;
	
	if(!warm_boot) {
		if(memctrl_desc->type == MEMCTRL_NONE || board_desc->memctrl_memsize > 0) {
			if(!memctrl_desc->plltest_required || !memctrl_desc->plltest_function(1)) {
				if(memctrl_desc->initialization_required) {

					printf("I: Initializing %s\n", memctrl_desc->name); 
					memctrl_desc->initialization_function(1);

					if(memctrl_desc->cacheflush_required)
						memctrl_desc->cacheflush_function(1);
				}

				if(memctrl_desc->memtest_function(16*1024, 0) == 0)
					boot_sequence();
				else
					printf("E: Aborted boot on memory error\n");
			} else if(memctrl_desc->plltest_required) {
				printf("E: Faulty SDRAM clocking\n");
			}
		} else {
			printf("I: No SDRAM on this board, not booting\n");
			sdram_enabled = 0;
		}
	} else {
		printf("I: Warm boot\n");
		sdram_enabled = 0;
		boot_sequence();
	}

	if(board_desc->vga) {
		splash_showerr();
	}

	while(1) {
		putsnonl("\e[1mBIOS>\e[0m ");
		readstr(buffer, 64);
		do_command(buffer);
	}
	return 0;
}

