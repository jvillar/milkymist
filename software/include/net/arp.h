/*
 * Milkymist VJ SoC (Software)
 * Copyright 2010 José Ignacio Villar, ID2 Group, University of Seville, jose@dte.us.es
 * Copyright 2000, 2001 Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 *
 *	Based on Linux Monitor (LiMon)
 *
 */

/* **************************************************************************
 *
 * Revision History
 *
 * Revision 1.0  2010/01/01 00:02:00  jvillar
 * Ported to LM32 and Milkymist peripherals.
 * 
 * Revision 0.1  2009/01/17 12:04:03  jvillar
 * Code imported from ORPMON (openrisc 1200 monitor).
 *
 */


#ifndef __ARP_H__
#define __ARP_H__

/**********************************************************************/
/*
 *	Global functions and variables.
 */

extern int ArpTry;

extern void ArpRequest (void);	/* Send a ARP request */

/**********************************************************************/

#endif /* __ARP_H__ */

