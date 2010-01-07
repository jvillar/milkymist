/*
 * Milkymist VJ SoC (Software)
 * Copyright 2010 José Ignacio Villar, ID2 Group, University of Seville, jose@dte.us.es
 * Copyright 2000 Roland Borde
 * Copyright 2000 Paolo Scaffardi
 * Copyright 2000, 2001 Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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


/*
 * General Desription:
 *
 * The user interface supports commands for BOOTP, RARP, and TFTP.
 * Also, we support ARP internally. Depending on available data,
 * these interact as follows:
 *
 * BOOTP:
 *
 *	Prerequisites:	- own ethernet address
 *	We want:	- own IP address
 *			- TFTP server IP address
 *			- name of bootfile
 *	Next step:	ARP
 *
 * RARP:
 *
 *	Prerequisites:	- own ethernet address
 *	We want:	- own IP address
 *			- TFTP server IP address
 *	Next step:	ARP
 *
 * ARP:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *			- TFTP server IP address
 *	We want:	- TFTP server ethernet address
 *	Next step:	TFTP
 *
 * DHCP:
 *
 *     Prerequisites:   - own ethernet address
 *     We want:         - IP, Netmask, ServerIP, Gateway IP
 *                      - bootfilename, lease time
 *     Next step:       - TFTP
 *
 * TFTP:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *			- TFTP server IP address
 *			- TFTP server ethernet address
 *			- name of bootfile (if unknown, we use a default name
 *			  derived from our own IP address)
 *	We want:	- load the boot file
 *	Next step:	none
 */


#include <net/net.h>
#include <net/bootp.h>
#include <net/tftp.h>
#include <net/rarp.h>
#include <net/arp.h>
#include <netctrl.h>
#include <hw/eth.h>
#include <hal/time.h>
#include <string.h>
#include <stdio.h>

/** BOOTP EXTENTIONS **/

IPaddr_t	NetOurSubnetMask=0;		/* Our subnet mask (0=unknown)	*/
IPaddr_t	NetOurGatewayIP=0;		/* Our gateways IP address	*/
IPaddr_t	NetOurDNSIP=0;			/* Our DNS IP address		*/
char		NetOurNISDomain[32]={0,};	/* Our NIS domain		*/
char		NetOurHostName[32]={0,};	/* Our hostname			*/
char		NetOurRootPath[64]={0,};	/* Our bootpath			*/
unsigned short	NetBootFileSize=0;		/* Our bootfile size in blocks	*/

/** END OF BOOTP EXTENTIONS **/

unsigned long NetBootFileXferSize;		/* The actual transferred size of the bootfile (in bytes) */
unsigned char NetOurEther[6];			/* Our ethernet address */
unsigned char NetServerEther[6]; 		/* Boot server enet address */			
IPaddr_t NetOurIP;				/* Our IP addr (0 = unknown) */
IPaddr_t NetServerIP;				/* Our IP addr (0 = unknown) */
volatile unsigned char *NetRxPkt;		/* Current receive packet */
int NetRxPktLen;				/* Current rx packet length */
unsigned NetIPID;				/* IP packet ID */
unsigned char NetBcastAddr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }; /* Ethernet bcast address */
int NetState;					/* Network loop state */
char BootFile[128];				/* Boot File name */

volatile unsigned char	PktBuf[(PKTBUFSRX+1) * PKTSIZE_ALIGN + PKTALIGN]  __attribute__ ((section ("ETH_DATA")));

volatile unsigned char *NetRxPackets[PKTBUFSRX]  __attribute__ ((section ("ETH_DATA"))); /* Receive packets */

static rxhand_f *packetHandler;			/* Current RX packet handler */
static thand_f *timeHandler;			/* Current timeout handler */
static unsigned long	timeValue;		/* Current timeout value */
volatile unsigned char *NetTxPacket = 0;	/* THE transmit packet */

static int net_check_prereq (proto_t protocol);

/**********************************************************************/
/*
 *	Main network processing loop.
 */
int NetLoop(proto_t protocol)
{
	struct timestamp now;

	if (!NetTxPacket) {
		int i;

		/*
		 *	Setup packet buffers, aligned correctly.
		 */
#ifdef ET_DEBUG
		printf("NetTxPacket = 0x%08X\n", NetTxPacket);
#endif
		NetTxPacket = &PktBuf[0] + (PKTALIGN - 1);
#ifdef ET_DEBUG
		printf("NetTxPacket = 0x%08X\n", NetTxPacket);
#endif
		NetTxPacket -= (unsigned long)NetTxPacket % PKTALIGN;
#ifdef ET_DEBUG
		printf("NetTxPacket = 0x%08X\n", NetTxPacket);
#endif
		for (i = 0; i < PKTBUFSRX; i++) {
			NetRxPackets[i] = NetTxPacket + (i+1)*PKTSIZE_ALIGN;
#ifdef ET_DEBUG
			printf("NetRxPackets[%d] = 0x%08X\n", i, NetRxPackets[i]);
#endif
		}
	}

	printf("Halting ethernet\n");
	eth_halt();
	printf("initializing ethernet\n");
	eth_init(NetReceive); // eth_init(print_packet); //FIXED

restart:

	NetCopyEther(NetOurEther, (get_netctrl_desc()->eth_add));

	NetState = NETLOOP_CONTINUE;

	/*
	 *	Start the ball rolling with the given start function.  From
	 *	here on, this code is a state machine driven by received
	 *	packets and timer events.
	 */

	if (protocol == TFTP) {			/* TFTP */
		NetOurIP	= get_netctrl_desc()->ip;
		NetServerIP	= get_netctrl_desc()->srv_ip;
		NetOurGatewayIP = get_netctrl_desc()->gw_ip;
		NetOurSubnetMask= get_netctrl_desc()->mask;

		if (net_check_prereq (protocol) != 0) {
			return 0;
		}

		/* always use ARP to get server ethernet address */
 		ArpTry = 0;
		ArpRequest ();
	}

#if (CONFIG_COMMANDS & CFG_CMD_DHCP & 0)
	else if (protocol == DHCP) {
		if (net_check_prereq (protocol) != 0) {
			return 0;
		}

		/* Start with a clean slate... */
		NetOurIP = 0;
		NetServerIP = 0;
		DhcpRequest();		/* Basically same as BOOTP */
	}
#endif	/* CFG_CMD_DHCP */

	 else {				/* BOOTP or RARP */

		/*
                 * initialize our IP addr to 0 in order to accept ANY
                 * IP addr assigned to us by the BOOTP / RARP server
		 */
		NetOurIP = 0;
		NetServerIP = 0;

		if (net_check_prereq (protocol) != 0) {
			return 0;
		}
#ifdef BOOTP
		if (protocol == BOOTP) {
			BootpTry = 0;
			BootpRequest ();
		} 
#endif
#ifdef RARP
		if {
			RarpTry	 = 0;
			RarpRequest ();
		}
#endif

	}

	NetBootFileXferSize = 0;
	/*
	 *	Main packet reception loop.  Loop receiving packets until
	 *	someone sets `NetQuit'.
	 */
	for (;;) {
//		WATCHDOG_RESET();
		/*
		 *	Check the ethernet for a new packet.  The ethernet
		 *	receive routine will process it.
		 */
			eth_rx();

		/*
		 *	Abort if ctrl-c was pressed.
		 *
		if (ctrlc()) {
		    eth_halt();
			printf("\nAbort\n");
			return 0;
		}
		*/

		/*
		 *	Check for a timeout, and run the timeout handler
		 *	if we have one.
		 */
		time_get(&now);
		if (timeHandler && ((now.sec*1000+now.usec/1000) > timeValue)) { //FIXED
			thand_f *x;

			x = timeHandler;
			timeHandler = (thand_f *)0;
			(*x)();
		}


		switch (NetState) {

		case NETLOOP_RESTART:
			goto restart;

		case NETLOOP_SUCCESS:
			if (NetBootFileXferSize > 0) {
				printf("Bytes transferred = %ld (%lx hex)\n", NetBootFileXferSize, NetBootFileXferSize);
			}
			eth_halt();
			return NetBootFileXferSize;

		case NETLOOP_FAIL:
			return 0;
		}
	}
}

/**********************************************************************/


void NetStartAgain(void) {
	NetState = NETLOOP_RESTART;
}

/**********************************************************************/
/*
 *	Miscelaneous bits.
 */

void NetSetHandler(rxhand_f * f) {
	packetHandler = f;
}


void NetSetTimeout(int msecs, thand_f * f) {
	struct timestamp now;

	if (msecs == 0) {
		timeHandler = (thand_f *)0;
	} else {
		timeHandler = f;
		time_get(&now);
		timeValue = now.sec*1000+now.usec/1000 + msecs; 
	}
}


void NetSendPacket(volatile unsigned char * pkt, int len) {
	unsigned char *p;
	p = eth_get_tx_buf();

	memcpy(p, (void *) pkt, len);

	eth_send(p, len);
}


void NetReceive(volatile unsigned char * pkt, int len) {
	Ethernet_t *et;
	IP_t *ip;
	ARP_t *arp;
	int x;

	NetRxPkt = pkt;
	NetRxPktLen = len;
	et = (Ethernet_t *)pkt; // FIXED (allowed warning on casting with different alignments)

	x = et->et_protlen;

	if (x < 1514) {
		/*
		 *	Got a 802 packet.  Check the other protocol field.
		 */
		x = et->et_prot;
		ip = (IP_t *)(pkt + E802_HDR_SIZE); // FIXED (allowed warning on casting with different alignments)
		len -= E802_HDR_SIZE;
	} else {
		ip = (IP_t *)(pkt + ETHER_HDR_SIZE); // FIXED (allowed warning on casting with different alignments)
		len -= ETHER_HDR_SIZE;
	}

#ifdef ET_DEBUG
	printf("Receive from protocol 0x%x\n", x);
#endif

	switch (x) {

	case PROT_ARP:
		/*
		 * We have to deal with two types of ARP packets:
                 * - REQUEST packets will be answered by sending  our
                 *   IP address - if we know it.
                 * - REPLY packates are expected only after we asked
                 *   for the TFTP server's or the gateway's ethernet
                 *   address; so if we receive such a packet, we set
                 *   the server ethernet address
		 */
#ifdef ET_DEBUG
		printf("Got ARP\n");
#endif
		arp = (ARP_t *)ip;
		if (len < ARP_HDR_SIZE) {
			printf("bad length %d < %d\n", len, ARP_HDR_SIZE);
			return;
		}
		if (arp->ar_hrd != ARP_ETHER) {
			return;
		}
		if (arp->ar_pro != PROT_IP) {
			return;
		}
		if (arp->ar_hln != 6) {
			return;
		}
		if (arp->ar_pln != 4) {
			return;
		}

		if (NetOurIP == 0 || memcmp((void *) &arp->ar_data[16], (void *) &NetOurIP, 4)) {
			return;
		}

		switch (arp->ar_op) {
		case ARPOP_REQUEST:		/* reply with our IP address	*/
#ifdef ET_DEBUG
			printf("Got ARP REQUEST, return our IP\n");
#endif
			NetSetEther((unsigned char *)et, et->et_src, PROT_ARP);
			arp->ar_op = ARPOP_REPLY;
			NetCopyEther(&arp->ar_data[10], &arp->ar_data[0]);
			NetCopyEther(&arp->ar_data[0], NetOurEther);

			NetCopyIP(&arp->ar_data[16], &arp->ar_data[6]); 	
			NetCopyIP(&arp->ar_data[6], (unsigned char *) &NetOurIP);
			NetSendPacket((unsigned char *)et,((unsigned char *)arp-pkt)+ARP_HDR_SIZE);
			return;
		case ARPOP_REPLY:		/* set TFTP server eth addr	*/
#ifdef ET_DEBUG
			printf("Got ARP REPLY, set server/gtwy eth addr\n");
			//printf("&NetServerEther=0x%08X -> %02X:%02X:%02X:%02X:%02X:%02X; &arp->ar_data=0x%08X -> %02X:%02X:%02X:%02X:%02X:%02X \n", NetServerEther, NetServerEther[0], NetServerEther[1], NetServerEther[2], NetServerEther[3], NetServerEther[4], NetServerEther[5], &arp->ar_data[0], arp->ar_data[0], arp->ar_data[1], arp->ar_data[2], arp->ar_data[3], arp->ar_data[4], arp->ar_data[5]);
#endif
			NetCopyEther(NetServerEther, &arp->ar_data[0]);
			(*packetHandler)(0,0,0,0);	/* start TFTP */
			return;
		default:
#ifdef ET_DEBUG
			printf("Unexpected ARP opcode 0x%x\n", arp->ar_op);
#endif
			return;
		}

	case PROT_RARP:
#ifdef ET_DEBUG
		printf("Got RARP\n");
#endif
		arp = (ARP_t *)ip;
		if (len < ARP_HDR_SIZE) {
			printf("bad length %d < %d\n", len, ARP_HDR_SIZE);
			return;
		}

		if ((arp->ar_op != RARPOP_REPLY) ||
			(arp->ar_hrd != ARP_ETHER)   ||
			(arp->ar_pro != PROT_IP)     ||
			(arp->ar_hln != 6) || (arp->ar_pln != 4)) {

			printf("invalid RARP header\n");
		} else {
			NetCopyIP((unsigned char *) &NetOurIP, &arp->ar_data[16]);
			NetCopyIP((unsigned char *) &NetServerIP, &arp->ar_data[6]);
			NetCopyEther(NetServerEther, &arp->ar_data[0]);

			(*packetHandler)(0,0,0,0);
		}
		break;

	case PROT_IP:
#ifdef ET_DEBUG
		printf("Got IP\n");
#endif
		if (len < IP_HDR_SIZE) {
			printf ("len bad %d < %d\n", len, IP_HDR_SIZE);
			return;
		}
		if (len < ip->ip_len) {
			printf("len bad %d < %d\n", len, ip->ip_len);
			return;
		}
		len = ip->ip_len;
#ifdef ET_DEBUG
		printf("len=%d, v=%02x\n", len, ip->ip_hl_v & 0xff);
#endif
		if ((ip->ip_hl_v & 0xf0) != 0x40) {
			return;
		}

		if (ip->ip_off & 0x1fff) { /* Can't deal w/ fragments */
			return;
		}

		if (!NetCksumOk((unsigned short *)ip, IP_HDR_SIZE_NO_UDP)) {
			printf("checksum bad\n");
			return;
		}

		IPaddr_t packet_ip_dst; // FIXED (ip->ip_dst is unaligned to 32bits we will copy byte by byte)
		NetCopyIP((unsigned char *) &packet_ip_dst, (unsigned char *) &ip->ip_dst); //FIXED

		if (NetOurIP && packet_ip_dst != NetOurIP && packet_ip_dst != 0xFFFFFFFF) {
			return;
		}

		/*
		 * watch for ICMP host redirects
		 *
                 * There is no real handler code (yet). We just watch
                 * for ICMP host redirect messages. In case anybody
                 * sees these messages: please contact me
                 * (wd@denx.de), or - even better - send me the
                 * necessary fixes :-)
		 *
                 * Note: in all cases where I have seen this so far
                 * it was a problem with the router configuration,
                 * for instance when a router was configured in the
                 * BOOTP reply, but the TFTP server was on the same
                 * subnet. So this is probably a warning that your
                 * configuration might be wrong. But I'm not really
                 * sure if there aren't any other situations.
		 */
		if (ip->ip_p == IPPROTO_ICMP) {
			ICMP_t *icmph = (void *) &(ip->udp_src); // FIXED (warn on casting with different alignments)

			if (icmph->type != ICMP_REDIRECT)
				return;
			if (icmph->code != ICMP_REDIR_HOST)
				return;
			printf (" ICMP Host Redirect to ");
			IPaddr_t icmp_un_gateway;
			NetCopyIP((unsigned char *) &icmp_un_gateway, (unsigned char *) &icmph->un.gateway); // FIXED (icmph->un.gateway is unaligned to 32bits we will copy byte by byte)
			print_IPaddr(icmp_un_gateway);
			printf(" ");
		} else if (ip->ip_p != IPPROTO_UDP) {	/* Only UDP packets */
			return;
		}

		/*
		 *	IP header OK.  Pass the packet to the current handler.
		 */
		(*packetHandler)((unsigned char *)ip +IP_HDR_SIZE,
						ip->udp_dst,
						ip->udp_src,
						ip->udp_len - 8);

		break;
	}
}


/**********************************************************************/

static int net_check_prereq (proto_t protocol)
{
	switch (protocol) {
	case ARP:	/* nothing to do */
			break;

	case TFTP:
			if (NetServerIP == 0) {
			  printf	 ("*** ERROR: `serverip' not set\n");
				return (1);
			}

			if (NetOurIP == 0) {
				printf ("*** ERROR: `ipaddr' not set\n");
				return (1);
			}
			/* Fall through */

	case DHCP:
	case RARP:
	case BOOTP:
			if (memcmp(NetOurEther, "\0\0\0\0\0\0", 6) == 0) {
				printf ("*** ERROR: `ethaddr' not set\n");
				return (1);
			}
			/* Fall through */
	}
	return (0);	/* OK */
}
/**********************************************************************/

int
NetCksumOk(unsigned short * ptr, int len)
{
	return !((NetCksum((unsigned char *)ptr, len) + 1) & 0xfffe);
}


unsigned short NetCksum(unsigned char * ptr, int len)
{
	unsigned int xsum, i;
	
	xsum = 0;
	for (i=0;i<len;i=i+2){
		xsum = xsum + ((ptr[i]<<8)&0xFF00)+(ptr[i+1]&0xFF);
	}

	xsum = (xsum & 0xffff) + (xsum >> 16);
	xsum = (xsum & 0xffff) + (xsum >> 16);

	return (xsum & 0xffff);
}


void NetCopyEther(volatile unsigned char * to, unsigned char * from)
{
	int i;

	for (i = 0; i < 6; i++)
		*to++ = *from++;
}

void NetCopyIP(volatile unsigned char * to, unsigned char * from) // FIXED
{
	int i;

	for (i = 0; i < 4; i++)
		*to++ = *from++;
}

void
NetSetEther(volatile unsigned char * xet, unsigned char * addr, unsigned long prot)
{
	volatile Ethernet_t *et = (Ethernet_t *)xet; // FIXED (warn on casting with different alignments)

	NetCopyEther(et->et_dest, addr);
	NetCopyEther(et->et_src, NetOurEther);
	et->et_protlen = prot;
}


void
NetSetIP(volatile unsigned char * xip, IPaddr_t dest, int dport, int sport, int len)
{
	volatile IP_t *ip = (IP_t *)xip; // FIXED (allowed warning on casting with different alignments)

	/*
	 *	If the data is an odd number of bytes, zero the
	 *	byte after the last byte so that the checksum
	 *	will work.
	 */
	if (len & 1)
		xip[IP_HDR_SIZE + len] = 0;

	/*
	 *	Construct an IP and UDP header.
			(need to set no fragment bit - XXX)
	 */

	ip->ip_hl_v  = 0x45;		/* IP_HDR_SIZE / 4 (not including UDP) */
	ip->ip_tos   = 0;
	ip->ip_len   = IP_HDR_SIZE + len;
	ip->ip_id    = NetIPID++;
	ip->ip_off   = 0x4000;	/* No fragmentation */
	ip->ip_ttl   = 255;
	ip->ip_p     = 17;		/* UDP */
	ip->ip_sum   = 0;
	NetCopyIP((unsigned char *) &ip->ip_src, (unsigned char *) &NetOurIP);
	NetCopyIP((unsigned char *) &ip->ip_dst, (unsigned char *) &dest);
	ip->udp_src  = sport;
	ip->udp_dst  = dport;
	ip->udp_len  = 8 + len;
	ip->udp_xsum = 0;
	ip->ip_sum   = ~NetCksum((unsigned char *)ip, IP_HDR_SIZE_NO_UDP);
}

void copy_filename (unsigned char *dst, unsigned char *src, int size)
{
	if (*src && (*src == '"')) {
		++src;
		--size;
	}

	while ((--size > 0) && *src && (*src != '"')) {
		*dst++ = *src++;
	}
	*dst = '\0';
}

void ip_to_string (IPaddr_t x, char *s)
{
    char num[] = "0123456789ABCDEF";
    int i;

    x = x;
    
    for(i = 28; i >= 0; i -= 4)
      *s++ = num[((x >> i) & 0x0f)];
    *s = 0;
}

void print_IPaddr (IPaddr_t x) {
	char tmp[12];

	ip_to_string(x, tmp);

	printf(tmp);
}

static unsigned int i2a(char* dest,unsigned int x) {
	register unsigned int tmp=x;
	register unsigned int len=0;

	if (x>=100) { 
		*dest++=tmp/100+'0'; tmp=tmp%100; ++len;
	}

	if (x>=10) {
		*dest++=tmp/10+'0'; tmp=tmp%10; ++len;
	}

	*dest++=tmp+'0';

	return len+1;
}

char *inet_ntoa(unsigned long in) {
	static char buf[20];
	unsigned int len;
	unsigned char *ip=(unsigned char*)&in;

	len=i2a(buf,ip[0]); buf[len]='.'; ++len;
	len+=i2a(buf+ len,ip[1]); buf[len]='.'; ++len;
	len+=i2a(buf+ len,ip[2]); buf[len]='.'; ++len;
	len+=i2a(buf+ len,ip[3]); buf[len]=0;

	return buf;
}

unsigned long inet_aton(const char *cp) {
	unsigned long a[4];
	unsigned long ret;
	char *p = (char *)cp;
	int i,d;

	if (strcmp(cp, "255.255.255.255") == 0)
		return -1;
 
	for(i = 0; i < 4; i++) {
		a[i] = strtoul(p, 0, 0);
		for(d=1; (p[d] != '.') && (i < 3); d++);
		p = &p[d+1];
	}

	ret = (a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3]; 
	return ret;
}


