//////////////////////////////////////////////////////////////////////
////                                                              ////
////  eth_spram_256x32_nobyteselect.v                             ////
////                                                              ////
////  This file is part of a modified version of the Ethernet IP  ////
////  core project                                                ////
////  http://www.opencores.org/projects/ethmac/                   ////
////                                                              ////
////  Author(s):                                                  ////
////      - José Ignacio Villar (jose@dte.us.es)                  ////
////                                                              ////
////  All additional information is available in the Readme.txt   ////
////  file.                                                       ////
////                                                              ////
//////////////////////////////////////////////////////////////////////
////                                                              ////
//// Copyright (C) 2001, 2002 Authors                             ////
////                                                              ////
//// This source file may be used and distributed without         ////
//// restriction provided that this copyright statement is not    ////
//// removed from the file and that any derivative work contains  ////
//// the original copyright notice and the associated disclaimer. ////
////                                                              ////
//// This source file is free software; you can redistribute it   ////
//// and/or modify it under the terms of the GNU Lesser General   ////
//// Public License as published by the Free Software Foundation; ////
//// either version 2.1 of the License, or (at your option) any   ////
//// later version.                                               ////
////                                                              ////
//// This source is distributed in the hope that it will be       ////
//// useful, but WITHOUT ANY WARRANTY; without even the implied   ////
//// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      ////
//// PURPOSE.  See the GNU Lesser General Public License for more ////
//// details.                                                     ////
////                                                              ////
//// You should have received a copy of the GNU Lesser General    ////
//// Public License along with this source; if not, download it   ////
//// from http://www.opencores.org/lgpl.shtml                     ////
////                                                              ////
//////////////////////////////////////////////////////////////////////
//
// CVS Revision History
//
// $Log: eth_spram_256x32_nobyteselect.v,v $
//
//
// Revision 1.1  2010/01/02 18:47:09  jvillar
// modification to the original eth_spram_256x32 module. In this one single byte select signals
// are ignored to avoid wasting four brams when implemented in low bram count xilinx parts such
// as xc3s500e. We have to ensure that wishbone accesses are not made with byte selection words
// other than 4'b1111 or 4'b0000.
//
//
//

`include "eth_defines.v"
`include "timescale.v"

module eth_spram_256x32_nobyteselect(
	// Generic synchronous single-port RAM interface
	clk, rst, ce, we, oe, addr, di, do

`ifdef ETH_BIST
  ,
  // debug chain signals
  mbist_si_i,       // bist scan serial in
  mbist_so_o,       // bist scan serial out
  mbist_ctrl_i        // bist chain shift control
`endif



);

	//
	// Generic synchronous single-port RAM interface
	//
	input           clk;  // Clock, rising edge
	input           rst;  // Reset, active high
	input           ce;   // Chip enable input, active high
	input  [3:0]    we;   // Write enable input, active high
	input           oe;   // Output enable input, active high
	input  [7:0]    addr; // address bus inputs
	input  [31:0]   di;   // input data bus
	output [31:0]   do;   // output data bus


`ifdef ETH_BIST
  input   mbist_si_i;       // bist scan serial in
  output  mbist_so_o;       // bist scan serial out
  input [`ETH_MBIST_CTRL_WIDTH - 1:0] mbist_ctrl_i;       // bist chain shift control
`endif


	//
	// Generic single-port synchronous RAM model
	//

	//
	// Generic RAM's registers and wires
	//
	reg  [ 31: 0] mem [255:0]; // RAM content
	wire [31:0]  q;            // RAM output
	reg  [7:0]   raddr;        // RAM read address
	//
	// Data output drivers
	//
	assign do = (oe & ce) ? q : {32{1'bz}};

	//
	// RAM read and write
	//

	// read operation
	always@(posedge clk)
	  if (ce) // && !we)
		raddr <= #1 addr;    // read address needs to be registered to read clock

	assign #1 q = rst ? {32{1'b0}} : mem[raddr];

	// write operation
	always@(posedge clk)
        begin
		if (ce && (| we[0]))
			mem[addr] <= #1 di[ 31: 0];
        end

	// Task prints range of memory
	// *** Remember that tasks are non reentrant, don't call this task in parallel for multiple instantiations. 
	task print_ram;
	input [7:0] start;
	input [7:0] finish;
	integer rnum;
  	begin
    		for (rnum=start;rnum<=finish;rnum=rnum+1)
      			$display("Addr %h = %0h ",rnum,mem[rnum]);
  	end
	endtask

endmodule
