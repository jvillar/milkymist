/*
 * Milkymist VJ SoC (Software)
 * Copyright 2010 José Ignacio Villar, ID2 Group, University of Seville, jose@dte.us.es
 * Copyright 2000 Paolo Scaffardi
 * Copyright 1994 - 2000 Neil Russell.
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


#ifndef __BOOTP_H__
#define __BOOTP_H__

#ifndef __NET_H__
#include	"net.h"
#endif /* __NET_H__ */

/**********************************************************************/

/*
 *	BOOTP header.
 */
#if (CONFIG_COMMANDS & CFG_CMD_DHCP)
#define OPT_SIZE 312	/* Minimum DHCP Options size per RFC2131 - results in 576 byte pkt */
#else
#define OPT_SIZE 64
#endif

typedef struct
{
	unsigned char		bp_op;		/* Operation				*/
# define OP_BOOTREQUEST	1
# define OP_BOOTREPLY	2
	unsigned char		bp_htype;	/* Hardware type			*/
# define HWT_ETHER	1
	unsigned char		bp_hlen;	/* Hardware address length		*/
# define HWL_ETHER	6
	unsigned char		bp_hops;	/* Hop count (gateway thing)		*/
	unsigned long		bp_id;		/* Transaction ID			*/
	unsigned short		bp_secs;	/* Seconds since boot			*/
	unsigned short		bp_spare1;	/* Alignment				*/
	IPaddr_t	bp_ciaddr;	/* Client IP address			*/
	IPaddr_t	bp_yiaddr;	/* Your (client) IP address		*/
	IPaddr_t	bp_siaddr;	/* Server IP address			*/
	IPaddr_t	bp_giaddr;	/* Gateway IP address			*/
	unsigned char		bp_chaddr[16];	/* Client hardware address		*/
	char		bp_sname[64];	/* Server host name			*/
	char		bp_file[128];	/* Boot file name			*/
	char		bp_vend[OPT_SIZE];	/* Vendor information			*/
}	Bootp_t;

#define BOOTP_HDR_SIZE	sizeof (Bootp_t)
#define BOOTP_SIZE	(ETHER_HDR_SIZE + IP_HDR_SIZE + BOOTP_HDR_SIZE)

/**********************************************************************/
/*
 *	Global functions and variables.
 */

/* bootp.c */
extern unsigned long	BootpID;		/* ID of cur BOOTP request		*/
extern char	BootFile[128];		/* Boot file name			*/
extern int	BootpTry;
#ifdef CONFIG_BOOTP_RANDOM_DELAY
unsigned long		seed1, seed2;		/* seed for random BOOTP delay		*/
#endif


/* Send a BOOTP request */
extern void	BootpRequest (void);

/****************** DHCP Support *********************/
extern void DhcpRequest(void);

/* DHCP States */
typedef enum { INIT,
	       INIT_REBOOT,
	       REBOOTING,
	       SELECTING,
	       REQUESTING,
	       REBINDING,
	       BOUND,
	       RENEWING } dhcp_state_t;

#define DHCP_DISCOVER 1
#define DHCP_OFFER    2
#define DHCP_REQUEST  3
#define DHCP_DECLINE  4
#define DHCP_ACK      5
#define DHCP_NAK      6
#define DHCP_RELEASE  7

#define SELECT_TIMEOUT 3	/* Seconds to wait for offers */

/**********************************************************************/

#endif /* __BOOTP_H__ */
