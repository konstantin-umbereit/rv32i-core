/* rtl/regfile.sv
 *
 * Single-cycle RV32I register file
 */

module regfile #(
    parameter DATA_WIDTH = 32,
    parameter ADDR_WIDTH = 5     
) (
    input  logic                    clk,      /* rising edge */
    input  logic                    rst_n,    /* active-low asynchronous reset */
	
    /* Read ports (combinational) */
    input  logic [ADDR_WIDTH-1:0]   rs1,      /* source register 1 address */
    input  logic [ADDR_WIDTH-1:0]   rs2,      /* source register 2 address */
    output logic [DATA_WIDTH-1:0]   rd1,      /* data read from rs1 */
    output logic [DATA_WIDTH-1:0]   rd2,      /* data read from rs2 */
    
    /* Write ports (synchronous) */
    input  logic                    we,       /* write enable */
    input  logic [ADDR_WIDTH-1:0]   rd,       /* destination register address */
    input  logic [DATA_WIDTH-1:0]   wd        /* write data */
);
	/* Storage: 32 x 32-bit reisters */
    logic [DATA_WIDTH-1:0] regs [0:31];
    
    /* Write logic */
	always_ff @(posedge clk or negedge rst_n) begin
		if(!rst_n) begin
			for (int i = 0; i < 32; i++) begin
				regs[i] <= 32'b0;
			end
		end
		else if(we && rd != 0) begin
			regs[rd] <= wd;
		end
	end
	
	/* read logic (combinational) */
	assign rd1 = (rs1 == 0) ? '0 :regs[rs1];
	assign rd2 = (rs2 == 0) ? '0 :regs[rs2];
	
endmodule
