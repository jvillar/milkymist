/*
 * Milkymist VJ SoC (Software)
 * Copyright 2010 José Ignacio Villar, ID2 Group, University of Seville, jose@dte.us.es
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


#include <net/net.h>
#include <hal/time.h>
#include <string.h>
#include <stdio.h>
#include <board.h>


#define WELL_KNOWN_PORT	69		/* Well known TFTP port #		*/
#define TIMEOUT		2		/* Seconds to timeout for a lost pkt	*/
#define TIMEOUT_COUNT	10		/* # of timeouts before giving up	*/
					/* (for checking the image size)	*/
#define HASHES_PER_LINE	65		/* Number of "loading" hashes per line	*/

/*
 *	TFTP operations.
 */
#define TFTP_RRQ	1
#define TFTP_WRQ	2
#define TFTP_DATA	3
#define TFTP_ACK	4
#define TFTP_ERROR	5


static int	TftpServerPort;		/* The UDP port at their end		*/
static int	TftpOurPort;		/* The UDP port at our end		*/
static int	TftpTimeoutCount;
static unsigned	TftpBlock;
static unsigned	TftpLastBlock;
static int	TftpState;
#define STATE_RRQ	1
#define STATE_DATA	2
#define STATE_TOO_LARGE	3
#define STATE_BAD_MAGIC	4

char *tftp_filename;

static __inline__ void store_block (unsigned block, unsigned char * src, unsigned len) {
	unsigned long offset = block * 512;
	unsigned long newsize = offset + len;

	(void)memcpy((void *)(get_board_desc()->memctrl_membase + offset), src, len);

	if (NetBootFileXferSize < newsize)
		NetBootFileXferSize = newsize;
}

static void TftpSend (void);
static void TftpTimeout (void);

/**********************************************************************/

static void TftpSend (void) {
	volatile unsigned char * pkt;
	volatile unsigned char * xp;
	int len = 0;

	/*
	 *	We will always be sending some sort of packet, so
	 *	cobble together the packet headers now.
	 */
	pkt = NetTxPacket + ETHER_HDR_SIZE + IP_HDR_SIZE;

	switch (TftpState) {

		case STATE_RRQ:
			xp = pkt;
			*((unsigned short *) pkt) = TFTP_RRQ;
			pkt += sizeof(unsigned short);
			strcpy ((char *) pkt, tftp_filename);
			pkt += strlen(tftp_filename) + 1;
			strcpy ((char *) pkt, "octet");
			pkt += 5 /*strlen("octet")*/ + 1;
			len = pkt - xp;
			break;

		case STATE_DATA:
			xp = pkt;
			*((unsigned short *) pkt) = TFTP_ACK;
			pkt += sizeof(unsigned short);
			*((unsigned short *) pkt) = TftpBlock;
			pkt += sizeof(unsigned short);
			len = pkt - xp;
			break;

		case STATE_TOO_LARGE:
			xp = pkt;
			*((unsigned short *) pkt) = TFTP_ERROR;
			pkt += sizeof(unsigned short);
			*((unsigned short *) pkt) = 3;
			pkt += sizeof(unsigned short);
			strcpy ((char *)pkt, "File too large");
			pkt += 14 /*strlen("File too large")*/ + 1;
			len = pkt - xp;
			break;

		case STATE_BAD_MAGIC:
			xp = pkt;
			*((unsigned short *) pkt) = TFTP_ERROR;
			pkt += sizeof(unsigned short);
			*((unsigned short *) pkt) = 2;
			pkt += sizeof(unsigned short);
			strcpy ((char *) pkt, "File has bad magic");
			pkt += 18 /*strlen("File has bad magic")*/ + 1;
			len = pkt - xp;
			break;
	}

	NetSetEther(NetTxPacket, NetServerEther, PROT_IP);
	NetSetIP(NetTxPacket + ETHER_HDR_SIZE, NetServerIP, TftpServerPort, TftpOurPort, len);
	NetSendPacket(NetTxPacket, ETHER_HDR_SIZE + IP_HDR_SIZE + len);
}


static void TftpHandler (unsigned char * pkt, unsigned dest, unsigned src, unsigned len) {


	if (dest != TftpOurPort) {
		return;
	}
	if (TftpState != STATE_RRQ && src != TftpServerPort) {
		return;
	}

	if (len < 2) {
		return;
	}
	len -= 2;

	unsigned short val = *((unsigned short *) pkt);
	pkt += sizeof(unsigned short);

	switch (val) {

	case TFTP_RRQ:
	case TFTP_WRQ:
	case TFTP_ACK:
		break;
	default:
		break;

	case TFTP_DATA:
		if (len < 2)
			return;
		len -= 2;
		TftpBlock = *(unsigned short *)pkt;
		if (((TftpBlock - 1) % 10) == 0) {
			printf("#");
			TftpTimeoutCount = 0;
		} else if ((TftpBlock % (10 * HASHES_PER_LINE)) == 0) {
			printf ("\n\t ");
		}

		if (TftpState == STATE_RRQ) {
			TftpState = STATE_DATA;
			TftpServerPort = src;
			TftpLastBlock = 0;

			if (TftpBlock != 1) {	/* Assertion */
				printf ("\nTFTP error: "
					"First block is not block 1 (%d)\n"
					"Starting again\n\n",
					TftpBlock);
				NetStartAgain ();
				break;
			}
		}

		if (TftpBlock == TftpLastBlock) {
			/*
			 *	Same block again; ignore it.
			 */
			break;
		}

		TftpLastBlock = TftpBlock;
		NetSetTimeout (TIMEOUT * 1000, TftpTimeout);

		store_block (TftpBlock - 1, pkt + 2, len);

		/*
		 *	Acknoledge the block just received, which will prompt
		 *	the server for the next one.
		 */
		TftpSend ();

		if (len < 512) {
			/*
			 *	We received the whole thing.  Try to
			 *	run it.
			 */
			printf ("\ndone\n");
			NetState = NETLOOP_SUCCESS;
		}
		break;

	case TFTP_ERROR:
		printf ("\nTFTP error: '%s' (%d)\n", pkt + 2, *(unsigned short *)pkt);
		printf ("Starting again\n\n");
		NetStartAgain ();
		break;
	}
}


static void TftpTimeout (void)
{
	if (++TftpTimeoutCount >= TIMEOUT_COUNT) {
		printf ("\nRetry count exceeded; starting again\n");
		NetStartAgain ();
	} else {
		printf ("T ");
		NetSetTimeout (TIMEOUT * 1000, TftpTimeout);
		TftpSend ();
	}
}


void
TftpStart (void)
{
	struct timestamp now;
	time_get(&now);

#ifdef ET_DEBUG
	printf ("\nServer ethernet address %02x:%02x:%02x:%02x:%02x:%02x\n",
		NetServerEther[0],
		NetServerEther[1],
		NetServerEther[2],
		NetServerEther[3],
		NetServerEther[4],
		NetServerEther[5]
	);
#endif /* DEBUG */

	printf ("TFTP from server ");	print_IPaddr (NetServerIP);
	printf ("; our IP address is ");	print_IPaddr (NetOurIP);

	// Check if we need to send across this subnet
	if (NetOurGatewayIP && NetOurSubnetMask) {
	    IPaddr_t OurNet 	= NetOurIP    & NetOurSubnetMask;
	    IPaddr_t ServerNet 	= NetServerIP & NetOurSubnetMask;

	    if (OurNet != ServerNet) {
		printf ("; sending through gateway ");
		print_IPaddr (NetOurGatewayIP) ;
	    }
	}
	printf("\n");

	printf ("Filename '%s'.", tftp_filename);

	if (NetBootFileSize) {
	    printf (" Size is %d%s kB => %x Bytes", NetBootFileSize/2, (NetBootFileSize%2) ? ".5" : "",	NetBootFileSize<<9);
	}

	printf("\n");

	printf ("Load address: 0x%lx\n", get_board_desc()->memctrl_membase);

	printf ("Loading: *\b");

	NetSetTimeout (TIMEOUT * 1000, TftpTimeout);
	NetSetHandler (TftpHandler);

	TftpServerPort = WELL_KNOWN_PORT;
	TftpTimeoutCount = 0;
	TftpState = STATE_RRQ;
	TftpOurPort = 1024 + (now.usec % 3072); //FIXED

	TftpSend ();
}

void TftpSetFileName(char *filename) {
	tftp_filename = filename; 
}
