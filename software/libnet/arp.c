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
#include <net/tftp.h>
#include <net/arp.h>
#include <string.h>
#include <stdio.h>

#define TIMEOUT		5		/* Seconds before trying ARP again */
#define TIMEOUT_COUNT	1		/* # of timeouts before giving up  */

static void ArpHandler(unsigned char *pkt, unsigned dest, unsigned src, unsigned len);
static void ArpTimeout(void);

int	ArpTry = 0;

/*
 *	Handle a ARP received packet.
 */
static void ArpHandler(unsigned char *pkt, unsigned dest, unsigned src, unsigned len)
{
	/* Check if the frame is really an ARP reply */
	//printf("NetServerEther=0x%08X -> %02X:%02X:%02X:%02X:%02X:%02X; NetBcastAddr=0x%08X -> %02X:%02X:%02X:%02X:%02X:%02X \n", NetServerEther, NetServerEther[0], NetServerEther[1], NetServerEther[2], NetServerEther[3], NetServerEther[4], NetServerEther[5], NetBcastAddr, NetBcastAddr[0], NetBcastAddr[1], NetBcastAddr[2], NetBcastAddr[3], NetBcastAddr[4], NetBcastAddr[5]);
	if (memcmp (NetServerEther, NetBcastAddr, 6) != 0) {
#ifdef	DEBUG
		printf("Got good ARP - start TFTP\n");
#endif
		TftpStart ();
	} else {
#ifdef	DEBUG
		printf("Got bad ARP\n");
#endif
	}
}


/*
 *	Timeout on ARP request.
 */
static void ArpTimeout(void) //FIXED
{
	if (ArpTry >= TIMEOUT_COUNT) {
		printf("\nRetry count exceeded; starting again\n");
		NetStartAgain ();
	} else {
		NetSetTimeout (TIMEOUT * 1000, ArpTimeout);
		ArpRequest ();
	}
}


void ArpRequest (void)
{
	int i;
	volatile unsigned char *pkt;
	ARP_t *	arp;
#ifdef	DEBUG
	printf("ARP broadcast %d\n", ++ArpTry);
#endif	
	pkt = NetTxPacket; // Already aligned (valid cast)

	NetSetEther(pkt, NetBcastAddr, PROT_ARP);
	pkt += ETHER_HDR_SIZE;

	arp = (ARP_t *)pkt;

	arp->ar_hrd = ARP_ETHER;
	arp->ar_pro = PROT_IP;

	arp->ar_hln = 6;

	arp->ar_pln = 4;
	arp->ar_op  = ARPOP_REQUEST;
	NetCopyEther(&arp->ar_data[0], NetOurEther);			/* source ET addr	*/
	NetCopyIP(&arp->ar_data[6], (unsigned char *) &NetOurIP);	/* source IP addr	*/
	for (i=10; i<16; ++i) {
		arp->ar_data[i] = 0;					/* dest ET addr = 0	*/
	}

	if((NetServerIP & NetOurSubnetMask) != (NetOurIP & NetOurSubnetMask)) {
		NetCopyIP(&arp->ar_data[16], (unsigned char *) &NetOurGatewayIP);
	} else {
		NetCopyIP(&arp->ar_data[16], (unsigned char *) &NetServerIP);
	}

	NetSendPacket(NetTxPacket, ETHER_HDR_SIZE + ARP_HDR_SIZE);
	NetSetTimeout(TIMEOUT * 1000, ArpTimeout);
	NetSetHandler(ArpHandler);



}

