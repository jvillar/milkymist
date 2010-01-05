/*
 * Milkymist VJ SoC
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

`include "setup.v"

module system(
	input clkin,
	input resetin,
	
	// Boot ROM
	output [23:0] flash_adr,
	input [7:0] flash_d,
	output flash_byte_n,
	output flash_oe_n,
	output flash_we_n,
	output flash_ce_n,

	// UART
	input uart_rxd,
	output uart_txd,

	// GPIO
	input [3:0] btn,     // 4
	input [3:0] dipsw,   // 4

	// SDRAM
	output sdram_clk,
	output sdram_cke,
	output sdram_cs_n,
	output sdram_we_n,
	output sdram_cas_n,
	output sdram_ras_n,
	output sdram_dqm,
	output [12:0] sdram_address,
	output [1:0] sdram_ba,
	inout [15:0] sdram_dq,

	// Ethernet
	output eth_tx_er,
	input eth_tx_clk,
	output eth_tx_en,
	output [3:0] eth_txd,
	input eth_rx_er,
	input eth_rx_clk,
	input eth_rx_dv,
	input [3:0] eth_rxd,
	input eth_col,
	input eth_crs,
	output eth_trste,
	input eth_fds_mdint,
	inout eth_mdio,
	output eth_mdc
);

//------------------------------------------------------------------
// Clock and Reset Generation
//------------------------------------------------------------------
wire sys_clk;

`ifndef SIMULATION

BUFG clkbuf(
	.I(clkin),
	.O(sys_clk)
);

`else

assign sys_clk = clkin;

`endif

`ifndef SIMULATION

/* Synchronize and negate the reset input */
reg rst0;
reg rst1;
always @(posedge sys_clk) rst0 <= ~resetin;
always @(posedge sys_clk) rst1 <= rst0;

/* Debounce it (counter holds reset for 10.49ms),
 * and generate power-on reset.
 */
reg [19:0] rst_debounce;
reg sys_rst;
initial rst_debounce <= 20'hFFFFF;
initial sys_rst <= 1'b1;
always @(posedge sys_clk) begin
	if(~rst1) /* reset is active low */
		rst_debounce <= 20'hFFFFF;
	else if(rst_debounce != 20'd0)
		rst_debounce <= rst_debounce - 20'd1;
	sys_rst <= rst_debounce != 20'd0;
end

`else

wire sys_rst;
assign sys_rst = ~resetin;

`endif

//------------------------------------------------------------------
// Wishbone master wires
//------------------------------------------------------------------
wire [31:0]	cpuibus_adr,
		cpudbus_adr,
		eth_wm_adr;

wire [2:0]	cpuibus_cti,
		cpudbus_cti;

wire [31:0]	cpuibus_dat_r,
		cpudbus_dat_r,
		cpudbus_dat_w,
		eth_wm_dat_r,
		eth_wm_dat_w;

wire [3:0]	cpudbus_sel,
		eth_wm_sel;

wire		cpudbus_we,
		eth_wm_we;

wire		cpuibus_cyc,
		cpudbus_cyc,
		eth_wm_cyc;

wire		cpuibus_stb,
		cpudbus_stb,
		eth_wm_stb;

wire		cpuibus_ack,
		cpudbus_ack,
		eth_wm_ack,
		eth_wm_err;

//------------------------------------------------------------------
// Wishbone slave wires
//------------------------------------------------------------------
wire [31:0]	norflash_adr,
		bram_adr,
		csrbrg_adr,
		sdram_adr,
		eth_ws_adr;

wire [2:0]	bram_cti;

wire [31:0]	norflash_dat_r,
		bram_dat_r,
		bram_dat_w,
		csrbrg_dat_r,
		csrbrg_dat_w,
		sdram_dat_r,
		sdram_dat_w,
		eth_ws_dat_r,
		eth_ws_dat_w;

wire [3:0]	bram_sel,
		sdram_sel,
		eth_ws_sel;

wire		bram_we,
		csrbrg_we,
		sdram_we,
		eth_ws_we;

wire		brg_cyc,
		norflash_cyc,
		bram_cyc,
		csrbrg_cyc,
		sdram_cyc,
		eth_ws_cyc;

wire		norflash_stb,
		bram_stb,
		csrbrg_stb,
		sdram_stb,
		eth_ws_stb;

wire		norflash_ack,
		bram_ack,
		csrbrg_ack,
		sdram_ack,
		eth_ws_ok,
		eth_ws_err,
		eth_ws_ack;

//---------------------------------------------------------------------------
// Wishbone switch
//---------------------------------------------------------------------------
conbus #(
	.s_addr_w(3),
	.s0_addr(3'b000),	// norflash	0x00000000
	.s1_addr(3'b001),	// bram		0x20000000
	.s2_addr(3'b010),	// sdram	0x40000000
	.s3_addr(3'b100),	// CSR bridge	0x80000000
	.s4_addr(3'b101)	// ethernet	0xa0000000
) conbus (
	.sys_clk(sys_clk),
	.sys_rst(sys_rst),

	// Master 0
	.m0_dat_i(32'hx),
	.m0_dat_o(cpuibus_dat_r),
	.m0_adr_i(cpuibus_adr),
	.m0_cti_i(cpuibus_cti),
	.m0_we_i(1'b0),
	.m0_sel_i(4'hf),
	.m0_cyc_i(cpuibus_cyc),
	.m0_stb_i(cpuibus_stb),
	.m0_ack_o(cpuibus_ack),
	// Master 1
	.m1_dat_i(cpudbus_dat_w),
	.m1_dat_o(cpudbus_dat_r),
	.m1_adr_i(cpudbus_adr),
	.m1_cti_i(cpudbus_cti),
	.m1_we_i(cpudbus_we),
	.m1_sel_i(cpudbus_sel),
	.m1_cyc_i(cpudbus_cyc),
	.m1_stb_i(cpudbus_stb),
	.m1_ack_o(cpudbus_ack),
	// Master 2
	.m2_dat_i(eth_wm_dat_w),
	.m2_dat_o(eth_wm_dat_r),
	.m2_adr_i(eth_wm_adr),
	.m2_cti_i(3'd0),
	.m2_we_i(eth_wm_we),
	.m2_sel_i(eth_wm_sel),
	.m2_cyc_i(eth_wm_cyc),
	.m2_stb_i(eth_wm_stb),
	.m2_ack_o(eth_wm_ack),
	// Master 3
	.m3_dat_i(32'b0),
	.m3_adr_i(32'b0),
	.m3_cti_i(3'b0),
	.m3_we_i(1'b0),
	.m3_sel_i(4'b0),
	.m3_cyc_i(1'b0),
	.m3_stb_i(1'b0),
	// Master 4
	.m4_dat_i(32'b0),
	.m4_adr_i(32'b0),
	.m4_cti_i(3'b0),
	.m4_we_i(1'b0),
	.m4_sel_i(4'b0),
	.m4_cyc_i(1'b0),
	.m4_stb_i(1'b0),

	// Slave 0
	.s0_dat_i(norflash_dat_r),
	.s0_adr_o(norflash_adr),
	.s0_cyc_o(norflash_cyc),
	.s0_stb_o(norflash_stb),
	.s0_ack_i(norflash_ack),
	// Slave 1
	.s1_dat_i(bram_dat_r),
	.s1_dat_o(bram_dat_w),
	.s1_adr_o(bram_adr),
	.s1_cti_o(bram_cti),
	.s1_sel_o(bram_sel),
	.s1_we_o(bram_we),
	.s1_cyc_o(bram_cyc),
	.s1_stb_o(bram_stb),
	.s1_ack_i(bram_ack),
	// Slave 2
	.s2_dat_i(sdram_dat_r),
	.s2_dat_o(sdram_dat_w),
	.s2_adr_o(sdram_adr),
	.s2_sel_o(sdram_sel),
	.s2_we_o(sdram_we),
	.s2_cyc_o(sdram_cyc),
	.s2_stb_o(sdram_stb),
	.s2_ack_i(sdram_ack),
	// Slave 3
	.s3_dat_i(csrbrg_dat_r),
	.s3_dat_o(csrbrg_dat_w),
	.s3_adr_o(csrbrg_adr),
	.s3_we_o(csrbrg_we),
	.s3_cyc_o(csrbrg_cyc),
	.s3_stb_o(csrbrg_stb),
	.s3_ack_i(csrbrg_ack),
	// Slave 4
	.s4_dat_i(eth_ws_dat_r),
	.s4_dat_o(eth_ws_dat_w),
	.s4_adr_o(eth_ws_adr),
	.s4_sel_o(eth_ws_sel),
	.s4_we_o(eth_ws_we),
	.s4_cyc_o(eth_ws_cyc),
	.s4_stb_o(eth_ws_stb),
	.s4_ack_i(eth_ws_ack)
);

//------------------------------------------------------------------
// CSR bus
//------------------------------------------------------------------
wire [13:0]	csr_a;
wire		csr_we;
wire [31:0]	csr_dw;
wire [31:0]	csr_dr_uart,
		csr_dr_sysctl,
		csr_dr_sdram;

//---------------------------------------------------------------------------
// WISHBONE to CSR bridge
//---------------------------------------------------------------------------
csrbrg csrbrg(
	.sys_clk(sys_clk),
	.sys_rst(sys_rst),
	
	.wb_adr_i(csrbrg_adr),
	.wb_dat_i(csrbrg_dat_w),
	.wb_dat_o(csrbrg_dat_r),
	.wb_cyc_i(csrbrg_cyc),
	.wb_stb_i(csrbrg_stb),
	.wb_we_i(csrbrg_we),
	.wb_ack_o(csrbrg_ack),
	
	.csr_a(csr_a),
	.csr_we(csr_we),
	.csr_do(csr_dw),
	/* combine all slave->master data lines with an OR */
	.csr_di(
		 csr_dr_uart
		|csr_dr_sysctl
		|csr_dr_sdram
	)
);

//---------------------------------------------------------------------------
// Interrupts
//---------------------------------------------------------------------------
wire gpio_irq;
wire timer0_irq;
wire timer1_irq;
wire uartrx_irq;
wire uarttx_irq;
wire eth_irq;

wire [31:0] cpu_interrupt;
assign cpu_interrupt = {19'd0,
	eth_irq,
	1'b0,
	1'b0,
	1'b0,
	1'b0,
	1'b0,
	1'b0,
	1'b0,
	uarttx_irq,
	uartrx_irq,
	timer1_irq,
	timer0_irq,
	gpio_irq
};

//---------------------------------------------------------------------------
// LM32 CPU
//---------------------------------------------------------------------------
lm32_top cpu(
	.clk_i(sys_clk),
	.rst_i(sys_rst),
	.interrupt(cpu_interrupt),

	.I_ADR_O(cpuibus_adr),
	.I_DAT_I(cpuibus_dat_r),
	.I_DAT_O(),
	.I_SEL_O(),
	.I_CYC_O(cpuibus_cyc),
	.I_STB_O(cpuibus_stb),
	.I_ACK_I(cpuibus_ack),
	.I_WE_O(),
	.I_CTI_O(cpuibus_cti),
	.I_LOCK_O(),
	.I_BTE_O(),
	.I_ERR_I(1'b0),
	.I_RTY_I(1'b0),

	.D_ADR_O(cpudbus_adr),
	.D_DAT_I(cpudbus_dat_r),
	.D_DAT_O(cpudbus_dat_w),
	.D_SEL_O(cpudbus_sel),
	.D_CYC_O(cpudbus_cyc),
	.D_STB_O(cpudbus_stb),
	.D_ACK_I(cpudbus_ack),
	.D_WE_O (cpudbus_we),
	.D_CTI_O(cpudbus_cti),
	.D_LOCK_O(),
	.D_BTE_O(),
	.D_ERR_I(1'b0),
	.D_RTY_I(1'b0)
);

//---------------------------------------------------------------------------
// Boot ROM
//---------------------------------------------------------------------------
norflash8 #(
	.adr_width(24),
	.swapbytes(1'b0)
) norflash (
	.sys_clk( sys_clk ),
	.sys_rst( sys_rst ),

	.wb_adr_i(norflash_adr),
	.wb_dat_o(norflash_dat_r),
	.wb_stb_i(norflash_stb),
	.wb_cyc_i(norflash_cyc),
	.wb_ack_o(norflash_ack),
	
	.flash_adr(flash_adr),
	.flash_d(flash_d)
);

assign flash_byte_n = 1'b0;
assign flash_oe_n = 1'b0;
assign flash_we_n = 1'b1;
assign flash_ce_n = 1'b0;




//---------------------------------------------------------------------------
// BRAM
//---------------------------------------------------------------------------
bram #(
	.adr_width(12)
) bram (
	.sys_clk(sys_clk),
	.sys_rst(sys_rst),

	.wb_adr_i(bram_adr),
	.wb_dat_o(bram_dat_r),
	.wb_dat_i(bram_dat_w),
	.wb_sel_i(bram_sel),
	.wb_stb_i(bram_stb),
	.wb_cyc_i(bram_cyc),
	.wb_ack_o(bram_ack),
	.wb_we_i(bram_we)
);


//---------------------------------------------------------------------------
// sdram
//---------------------------------------------------------------------------

wire [1:0] sdram_dqm_all;
assign sdram_dqm = sdram_dqm_all[0];
yadmc #(
	.csr_addr(4'h2),
	.phase_shift(`SDRAM_PHASE_SHIFT),
	.clk_multiply(`SDRAM_CLK_MULTIPLY),
	.clk_divide(`SDRAM_CLK_DIVIDE)
) sdram (
	.sys_clk( sys_clk ),
	.sys_rst( sys_rst ),

	.csr_a(csr_a),
	.csr_we(csr_we),
	.csr_di(csr_dw),
	.csr_do(csr_dr_sdram),

	.wb_adr_i(sdram_adr),
	.wb_dat_i(sdram_dat_w),
	.wb_dat_o(sdram_dat_r),
	.wb_sel_i(sdram_sel),
	.wb_cyc_i(sdram_cyc),
	.wb_stb_i(sdram_stb),
	.wb_we_i(sdram_we),
	.wb_ack_o(sdram_ack),

	/* SDRAM interface */
	.ext_sdram_clk(sdram_clk),
	.sdram_cke(sdram_cke),
	.sdram_cs_n(sdram_cs_n),
	.sdram_we_n(sdram_we_n),
	.sdram_cas_n(sdram_cas_n),
	.sdram_ras_n(sdram_ras_n),
	.sdram_dqm(sdram_dqm_all),
	.sdram_adr(sdram_address),
	.sdram_ba(sdram_ba),
	.sdram_dq(sdram_dq)

);


//---------------------------------------------------------------------------
// UART
//---------------------------------------------------------------------------
uart #(
	.csr_addr(4'h0),
	.clk_freq(`CLOCK_FREQUENCY),
	.baud(`BAUD_RATE)
) uart (
	.sys_clk(sys_clk),
	.sys_rst(sys_rst),

	.csr_a(csr_a),
	.csr_we(csr_we),
	.csr_di(csr_dw),
	.csr_do(csr_dr_uart),
	
	.rx_irq(uartrx_irq),
	.tx_irq(uarttx_irq),
	
	.uart_rxd(uart_rxd),
	.uart_txd(uart_txd)
);


//---------------------------------------------------------------------------
// System Controller
//---------------------------------------------------------------------------
wire [13:0] gpio_outputs;

sysctl #(
	.csr_addr(4'h1),
	.ninputs(13),
	.noutputs(14),
	.systemid(32'h5333534B) /* X401 */
) sysctl (
	.sys_clk(sys_clk),
	.sys_rst(sys_rst),

	.gpio_irq(gpio_irq),
	.timer0_irq(timer0_irq),
	.timer1_irq(timer1_irq),

	.csr_a(csr_a),
	.csr_we(csr_we),
	.csr_di(csr_dw),
	.csr_do(csr_dr_sysctl),

	.gpio_inputs({dipsw, btn}),
	.gpio_outputs(gpio_outputs)
);


//---------------------------------------------------------------------------
// Ethernet 10/100 MAC
//---------------------------------------------------------------------------
wire eth_mdo;
wire eth_mdoe;

assign eth_mdio = eth_mdoe ? eth_mdo : 1'bz;
assign eth_trste = 1'b0;
assign eth_ws_ack = eth_ws_ok | eth_ws_err;
assign eth_wm_err = 1'b0;

eth_top eth_top (

	// WISHBONE common
	.wb_clk_i( sys_clk ),
	.wb_rst_i( sys_rst ),
	
	// WISHBONE slave
	.wb_dat_i(eth_ws_dat_w),
	.wb_dat_o(eth_ws_dat_r),
	.wb_adr_i(eth_ws_adr[11:2]),
	.wb_sel_i(eth_ws_sel),
	.wb_we_i(eth_ws_we),
	.wb_cyc_i(eth_ws_cyc),
	.wb_stb_i(eth_ws_stb),
	.wb_ack_o(eth_ws_ok),
	.wb_err_o(eth_ws_err), 

	// WISHBONE master
	.m_wb_adr_o(eth_wm_adr),
	.m_wb_sel_o(eth_wm_sel),
	.m_wb_we_o(eth_wm_we), 
	.m_wb_dat_o(eth_wm_dat_w),
	.m_wb_dat_i(eth_wm_dat_r),
	.m_wb_cyc_o(eth_wm_cyc), 
	.m_wb_stb_o(eth_wm_stb),
	.m_wb_ack_i(eth_wm_ack),
	.m_wb_err_i(eth_wm_err), 

	// TX
	.mtx_clk_pad_i(eth_tx_clk),
	.mtxd_pad_o(eth_txd),
	.mtxen_pad_o(eth_tx_en),
	.mtxerr_pad_o(eth_tx_er),

	// RX
	.mrx_clk_pad_i(eth_rx_clk),
	.mrxd_pad_i(eth_rxd),
	.mrxdv_pad_i(eth_rx_dv),
	.mrxerr_pad_i(eth_rx_er),
	.mcoll_pad_i(eth_col),
	.mcrs_pad_i(eth_crs),
  
	// MIIM
	.mdc_pad_o(eth_mdc),
	.md_pad_i(eth_mdio),
	.md_pad_o(eth_mdo),
	.md_padoe_o(eth_mdoe),

	// Interrupt
	.int_o(eth_irq)
);

endmodule
