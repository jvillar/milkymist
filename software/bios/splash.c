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
#include <system.h>
#include <board.h>
#include <memctrl.h>
#include <hw/vga.h>

#include "splash.h"

static int splash_hres;
static int splash_vres;
static unsigned short *splash_fb = NULL;

void splash_display(void *fbadr)
{

	if( !get_board_desc()->vga ) {
		printf("E: No VGA support on this board\n");
		return;
	}

	int i;
	unsigned short *splash_src = (unsigned short *)65536;

	printf("I: Displaying splash screen...");

	splash_fb = (unsigned short *)fbadr;
	splash_hres = 640;
	splash_vres = 480;

	for(i=0;i<splash_hres*splash_vres;i++)
		splash_fb[i] = splash_src[i];

	if( !get_memctrl_desc()->cacheflush_required ) {
		get_memctrl_desc()->cacheflush_function(0);
	}

	CSR_VGA_BASEADDRESS = (unsigned int)splash_fb;
	CSR_VGA_RESET = 0;

	printf("OK\n");
}

void splash_showerr()
{
	if( !get_board_desc()->vga ) {
		printf("E: No VGA support on this board\n");
		return;
	}

	int x, y;
	unsigned short color = 0xF800;

	if(splash_fb == NULL) return;
	for(y=0;y<5;y++)
		for(x=0;x<splash_hres;x++)
			splash_fb[splash_hres*y+x] = color;
	for(;y<(splash_vres-5);y++) {
		for(x=0;x<5;x++)
			splash_fb[splash_hres*y+x] = color;
		for(x=splash_hres-5;x<splash_hres;x++)
			splash_fb[splash_hres*y+x] = color;
	}
	for(;y<splash_vres;y++)
		for(x=0;x<splash_hres;x++)
			splash_fb[splash_hres*y+x] = color;

	if( !get_memctrl_desc()->cacheflush_required ) {
		get_memctrl_desc()->cacheflush_function(0);
	}
}
