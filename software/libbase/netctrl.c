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

#include <netctrl.h>
#include <board.h>
#include <stdlib.h>
#include <hw/eth.h>

struct netctrl_desc netctrl;
int netctrl_initialized;

static struct netctrl_desc netctrls[2] = {
	{
		.type = NETCTRL_NONE,
		.name = "no network controller",
		.eth_add = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
		.ip = 0,
		.gw_ip = 0,
		.mask = 0,
		.srv_ip = 0,

		.init_function = NULL,
		.halt_function = NULL,
		.receive_function = NULL,
		.send_function = NULL 
	},
	{
		.type = OPENCORES_ETH_10_100,
		.name = "Opencores 10/100 Eth Controller",

		.eth_add = { 0x00, 0x12, 0x34, 0x56, 0x78, 0x9a }, // 02:00:00:00:00:02
		.ip = 0xC0A80002, // 192.168.0.2
		.gw_ip = 0xC0A80001, // 192.168.0.1
		.mask = 0xFFFFFF00,
		.srv_ip = 0xC0A80001, // 192.168.0.1

		.init_function = eth_init,
		.halt_function = eth_halt,
		.receive_function = eth_rx,
		.send_function = eth_send 

	}
};

struct netctrl_desc *get_netctrl_desc_type(unsigned int type)
{
	unsigned int i;
	
	for(i=0;i<sizeof(netctrls)/sizeof(netctrls[0]);i++)
		if(netctrls[i].type == type) {
			netctrl = netctrls[i];
			return &netctrl;
		}
	return NULL;
}

struct netctrl_desc *get_netctrl_desc()
{
	if(netctrl_initialized != 1) {
		get_netctrl_desc_type(get_board_desc()->netctrl_type);
		netctrl_initialized = 1;
	}
	return &netctrl;
}

