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


#include <net/net.h>
#include <net/bootp.h>
#include <net/tftp.h>
#include <net/rarp.h>


#if (CONFIG_COMMANDS & CFG_CMD_NET)

#define TIMEOUT		5		/* Seconds before trying BOOTP again */
#define TIMEOUT_COUNT	1		/* # of timeouts before giving up    */


int		RarpTry;

/*
 *	Handle a RARP received packet.
 */
static void
RarpHandler(uchar * dummi0, unsigned dummi1, unsigned dummi2, unsigned dummi3)
{
#ifdef	DEBUG
	printf("Got good RARP\n");
#endif
	TftpStart ();
}


/*
 *	Timeout on BOOTP request.
 */
static void
RarpTimeout(void)
{
	if (RarpTry >= TIMEOUT_COUNT) {
		puts ("\nRetry count exceeded; starting again\n");
		NetStartAgain ();
	} else {
		NetSetTimeout (TIMEOUT * 1000, RarpTimeout);
		RarpRequest ();
	}
}


void
RarpRequest (void)
{
	int i;
	volatile uchar *pkt;
	ARP_t *	rarp;

	printf("RARP broadcast %d\n", ++RarpTry);
	pkt = NetTxPacket;

	NetSetEther(pkt, NetBcastAddr, PROT_RARP);
	pkt += ETHER_HDR_SIZE;

	rarp = (ARP_t *)pkt;

	rarp->ar_hrd = ARP_ETHER;
	rarp->ar_pro = PROT_IP;
	rarp->ar_hln = 6;
	rarp->ar_pln = 4;
	rarp->ar_op  = RARPOP_REQUEST;
	NetCopyEther(&rarp->ar_data[0], NetOurEther);			/* source ET addr */
	NetCopyIP(&arp->ar_data[6], (unsigned char *) &NetOurIP);	/* source IP addr */ // *(IPaddr_t *)(&rarp->ar_data[6]) = NetOurIP;
	NetCopyEther(&rarp->ar_data[10], NetOurEther);			/* dest ET addr = source ET addr ??*/
	/* dest. IP addr set to broadcast */
	for (i = 0; i <= 3; i++) {
		rarp->ar_data[16 + i] = 0xff;
	}

	NetSendPacket(NetTxPacket, ETHER_HDR_SIZE + ARP_HDR_SIZE);

	NetSetTimeout(TIMEOUT * 1000, RarpTimeout);
	NetSetHandler(RarpHandler);
}

#endif /* CFG_CMD_NET */
