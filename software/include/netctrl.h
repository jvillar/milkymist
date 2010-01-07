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

#ifndef __NETCTRL_H
#define __NETCTRL_H

#define NETCTRL_NAME_LEN 32

struct netctrl_desc {
	unsigned int type;
	char name[NETCTRL_NAME_LEN];

	unsigned char eth_add[6];
	unsigned int ip;
	unsigned int gw_ip;
	unsigned int mask;
	unsigned int srv_ip;


	void (*init_function)(void (*rec)(volatile unsigned char *, int));
	void (*halt_function)();
	unsigned int (*receive_function)();
	void (*send_function)(void *buf, unsigned int len);
};


struct netctrl_desc *get_netctrl_desc_type(unsigned int type);
struct netctrl_desc *get_netctrl_desc();

#endif /* __NETCTRL_H */
