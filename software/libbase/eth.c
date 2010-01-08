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

#include <hw/eth.h>
#include <stdio.h>


int tx_next;  /* Next buffer to be given to the user */
int tx_last;  /* Next buffer to be checked if packet sent */
int tx_full;
int rx_next;  /* Next buffer to be checked for new packet and given to the user */
void (*receive)(volatile unsigned char *add, int len); /* Pointer to function to be called when frame is received */

unsigned int eth_data[((ETH_TXBD_NUM + ETH_RXBD_NUM) * ETH_MAXBUF_LEN)/4]  __attribute__ ((section ("ETH_DATA"))) = {0};
#define ETH_DATA_BASE ((unsigned int) eth_data)


void print_packet(volatile unsigned char * add, int len) {

	int i;

	printf("ipacket: add = %lx len = %d\n", add, len);
	for(i = 0; i < len; i++) {
		if(!(i % 16))
			printf("\n");
		printf(" %.2x", *(add + i));
	}
	printf("\n");
}


void init_tx_bd_pool(void) {
	eth_bd  *bd;
	int i;
  
	bd = (eth_bd *) ETH_BD_BASE;

	for(i = 0; i < ETH_TXBD_NUM; i++) {
		// Set Tx BD status
		bd[i].len_status = 0 << 16 | ETH_TX_BD_PAD | ETH_TX_BD_CRC | ETH_RX_BD_IRQ;

		// Initialize Tx buffer pointer
		bd[i].addr = ETH_DATA_BASE + (i * ETH_MAXBUF_LEN);
#ifdef	DEBUG
		printf("TX POOL: bd[%d] = 0x%08X, bd[%d].len_status = 0x%08X, bd[%d].addr = 0x%08X\n", i, &bd[i], i, bd[i].len_status, i, bd[i].addr);
#endif	

	}

	bd[i-1].len_status |= ETH_TX_BD_WRAP; // Last Tx BD - Wrap
}


void init_rx_bd_pool(void)
{
	eth_bd  *bd;
	int i;

	bd = (eth_bd *) ETH_BD_BASE + ETH_TXBD_NUM;

	for(i = 0; i < ETH_RXBD_NUM; i++) {
		// Set Rx BD status
		bd[i].len_status = 0 << 16 | ETH_RX_BD_EMPTY | ETH_RX_BD_IRQ;

		// Initialize Rx buffer pointer
		bd[i].addr = ETH_DATA_BASE + ((ETH_TXBD_NUM + i) * ETH_MAXBUF_LEN);
#ifdef	DEBUG
		printf("RX POOL: bd[%d] = 0x%08X, bd[%d].len_status = 0x%08X, bd[%d].addr = 0x%08X\n", i, &bd[i], i, bd[i].len_status, i, bd[i].addr);
#endif	
	}


	bd[i-1].len_status |= ETH_RX_BD_WRAP; // Last Rx BD - Wrap
}


// Ethernet interrupt handler
void eth_int (void) {

}


void eth_init (void (*rec)(volatile unsigned char *, int)) {
	// Reset ethernet core 
	ETH_MODER  =  ETH_MODER_RST;  // Reset ON 
	ETH_MODER &= ~ETH_MODER_RST;  // Reset OFF

	// Setting TX BD number 
	ETH_TX_BD_NUM = ETH_TXBD_NUM;

	// Set PHY to 10 Mbps full duplex
	ETH_MIIADDRESS = 0<<8;
	ETH_MIITX_DATA = 0x0100;
	ETH_MIICOMMAND = ETH_MIICOMMAND_WCTRLDATA;

	while(ETH_MIISTATUS & ETH_MIISTATUS_BUSY);

	while(1) {
		ETH_MIIADDRESS = 1<<8;
		ETH_MIICOMMAND = ETH_MIICOMMAND_RSTAT;
		while(ETH_MIISTATUS & ETH_MIISTATUS_BUSY);
		if(ETH_MIIRX_DATA & 0x04)
			break;
	}

	// Set min/max packet length
	ETH_PACKETLEN = 0x00400600;

	// Set IPGT register to recomended value
	ETH_IPGT = 0x00000012;

	// Set IPGR1 register to recomended value 
	ETH_IPGR1 = 0x0000000c;

	// Set IPGR2 register to recomended value
	ETH_IPGR2 = 0x00000012;

	// Set COLLCONF register to recomended value 
	ETH_COLLCONF = 0x000f003f;

	ETH_CTRLMODER = 0;


	// Initialize RX and TX buffer descriptors 
	init_rx_bd_pool();

	init_tx_bd_pool();

	// Initialize tx pointers
	tx_next = 0;
	tx_last = 0;
	tx_full = 0;

	// Initialize rx pointers
	rx_next = 0;
	receive = rec;

	// Set local MAC address
	ETH_MAC_ADDR1 = ETH_MACADDR0 << 8 |
			ETH_MACADDR1;
	ETH_MAC_ADDR0 = ETH_MACADDR2 << 24 |
			ETH_MACADDR3 << 16 |
			ETH_MACADDR4 << 8 |
			ETH_MACADDR5;

	// Clear all pending interrupts
	ETH_INT = 0xffffffff;

	// Promisc, IFG, CRCEn
	ETH_MODER |= ETH_MODER_PAD | ETH_MODER_IFG | ETH_MODER_CRCEN;

	// Enable interrupt sources
	ETH_INT_MASK = 0x00000000;

	// Enable receiver and transmiter
	ETH_MODER |= ETH_MODER_RXEN | ETH_MODER_TXEN;

	// Register interrupt handler
	//int_add (ETH_IRQ, eth_int); //FIXME
}


// Returns pointer to next free buffer; NULL if none available
void *eth_get_tx_buf () {
	eth_bd  *bd;
	unsigned int add;

	if(tx_full)
		return (void *) 0;

	bd = (eth_bd *)ETH_BD_BASE;
  
	if(bd[tx_next].len_status & ETH_TX_BD_READY)
		return (void *) 0;

	add = bd[tx_next].addr;

	tx_next = (tx_next + 1) & ETH_TXBD_NUM_MASK;

	if(tx_next == tx_last)
		tx_full = 1;

	return (void *)add;
}

// Send a packet at address
void eth_send(void *pkt, unsigned int len) {
	unsigned char *p;
	p = eth_get_tx_buf();

	memcpy(p, (void *) pkt, len);

	eth_send_data(p, len);
}

// Send data at address
void eth_send_data (void *buf, unsigned int len) {
	eth_bd  *bd;


	bd = (eth_bd *) ETH_BD_BASE;

	bd[tx_last].addr = (unsigned int)buf;
	bd[tx_last].len_status &= 0x0000ffff & ~ETH_TX_BD_STATS;
	bd[tx_last].len_status |= len << 16 | ETH_TX_BD_READY;

	tx_last = (tx_last + 1) & ETH_TXBD_NUM_MASK;
	tx_full = 0;
}

/* Waits for packet and pass it to the upper layers */
unsigned int eth_rx (void)
{
	eth_bd  *bd;
	unsigned int len = 0;

	bd = (eth_bd *)ETH_BD_BASE + ETH_TXBD_NUM;
	while(1) {

		int bad = 0;

		if(bd[rx_next].len_status & ETH_RX_BD_EMPTY) {
#ifdef	DEBUG
			if(len > 0)
				printf("ETH RECEIVE PACKET LEN=%d\n",len);
#endif	
			return len;
		}
 
		if(bd[rx_next].len_status & ETH_RX_BD_OVERRUN) {
			printf("eth rx: ETH_RX_BD_OVERRUN\n");
			bad = 1;
		}
		if(bd[rx_next].len_status & ETH_RX_BD_INVSIMB) {
			printf("eth rx: ETH_RX_BD_INVSIMB\n");
			bad = 1;
		}
		if(bd[rx_next].len_status & ETH_RX_BD_DRIBBLE) {
			printf("eth rx: ETH_RX_BD_DRIBBLE\n");
			bad = 1;
		}
		if(bd[rx_next].len_status & ETH_RX_BD_TOOLONG) {
			printf("eth rx: ETH_RX_BD_TOOLONG\n");
			bad = 1;
		}
		if(bd[rx_next].len_status & ETH_RX_BD_SHORT) {
			printf("eth rx: ETH_RX_BD_SHORT\n");
			bad = 1;
		}
		if(bd[rx_next].len_status & ETH_RX_BD_CRCERR) {
			printf("eth rx: ETH_RX_BD_CRCERR\n");
			bad = 1;
		}
		if(bd[rx_next].len_status & ETH_RX_BD_LATECOL) {
			printf("eth rx: ETH_RX_BD_LATECOL\n");
			bad = 1;
		}

		if(!bad) {
			receive((void *)bd[rx_next].addr, bd[rx_next].len_status >> 16); 
			len += bd[rx_next].len_status >> 16;
		}

		bd[rx_next].len_status &= ~ETH_RX_BD_STATS;
		bd[rx_next].len_status |= ETH_RX_BD_EMPTY;

		rx_next = (rx_next + 1) & ETH_RXBD_NUM_MASK;
	}
}


void eth_int_enable(void) {
	ETH_INT_MASK =  ETH_INT_MASK_TXB	|
			ETH_INT_MASK_TXE	|
			ETH_INT_MASK_RXF	|
			ETH_INT_MASK_RXE	|
			ETH_INT_MASK_BUSY	|
			ETH_INT_MASK_TXC	|
			ETH_INT_MASK_RXC;
}


void eth_halt(void) {
	// Enable receiver and transmiter
	ETH_MODER &= ~(ETH_MODER_RXEN | ETH_MODER_TXEN);
}

