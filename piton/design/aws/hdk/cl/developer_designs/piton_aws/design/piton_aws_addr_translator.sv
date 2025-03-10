// ========== Copyright Header Begin ============================================
// Copyright (c) 2019 Princeton University
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Princeton University nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY PRINCETON UNIVERSITY "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL PRINCETON UNIVERSITY BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ========== Copyright Header End ============================================

`include "noc_axi4_bridge_define.vh"

module piton_aws_addr_translator (
	axi_bus_t.master in, 
	axi_bus_t.slave out
);

	logic [`AXI4_ADDR_WIDTH-1:0] translated_awaddr = (in.awaddr >= 64'hfff0000000) ? in.awaddr - 64'hfff0000000 + 64'h200000000 
													   : (in.awaddr >= 64'hf000000000) ? in.awaddr - 64'hf000000000 + 64'h200000000 
													   : in.awaddr;
													   
	logic [`AXI4_ADDR_WIDTH-1:0] translated_araddr = (in.araddr >= 64'hfff0000000) ? in.araddr - 64'hfff0000000 + 64'h200000000 
													   : (in.araddr >= 64'hf000000000) ? in.araddr - 64'hf000000000 + 64'h200000000 
													   : in.araddr;

	assign out.awid = in.awid;
	assign out.awaddr = translated_awaddr;
	assign out.awlen = in.awlen;
	assign out.awsize = in.awsize;
	assign out.awburst = in.awburst;
	assign out.awlock = in.awlock;
	assign out.awcache = in.awcache;
	assign out.awprot = in.awprot;
	assign out.awqos = in.awqos;
	assign out.awregion = in.awregion;
	assign out.awuser = in.awuser;
	assign out.awvalid = in.awvalid;
	assign in.awready = out.awready;

	assign out.wid = in.wid;
	assign out.wdata = in.wdata;
	assign out.wstrb = in.wstrb;
	assign out.wlast = in.wlast;
	assign out.wuser = in.wuser;
	assign out.wvalid = in.wvalid;
	assign in.wready = out.wready;

	assign in.bid = out.bid;
	assign in.bresp = out.bresp;
	assign in.buser = out.buser;
	assign in.bvalid = out.bvalid;
	assign out.bready = in.bready;

	assign out.arid = in.arid;
	assign out.araddr = translated_araddr;
	assign out.arlen = in.arlen;
	assign out.arsize = in.arsize;
	assign out.arburst = in.arburst;
	assign out.arlock = in.arlock;
	assign out.arcache = in.arcache;
	assign out.arprot = in.arprot;
	assign out.arqos = in.arqos;
	assign out.arregion = in.arregion;
	assign out.aruser = in.aruser;
	assign out.arvalid = in.arvalid;
	assign in.arready = out.arready;

	assign in.rid = out.rid;
	assign in.rdata = out.rdata;
	assign in.rresp = out.rresp;
	assign in.rlast = out.rlast;
	assign in.ruser = out.ruser;
	assign in.rvalid = out.rvalid;
	assign out.rready = in.rready;

endmodule // piton_aws_addr_translator

