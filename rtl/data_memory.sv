/* rtl/data_memory.sv
 *
 * Single-cycle RV32I data memory
 */

 module data_memory #(
    parameter DATA_WIDTH = 32,
    parameter ADDR_WIDTH = 32,
    parameter MEM_SIZE = 1024
 ) (
    input  logic                  clk,          /* rising edge */ 
    input  logic                  rst_n,        /* active-low asynchronous reset */
    input  logic                  we,           /* write enable */
    input  logic [1:0]            data_mask,
    input  logic [ADDR_WIDTH-1:0] alu_result,   /* write destination */
    input  logic [DATA_WIDTH-1:0] wd,           /* write data */

    output logic [DATA_WIDTH-1:0] read_data
 );
    /* Memory: 1024 x 8bit */
    logic [7:0] mem [0:MEM_SIZE-1];

    /* Write logic */
    always_ff @(posedge clk or negedge rst_n) begin
        if(!rst_n)
            for (int i = 0; i < MEM_SIZE; i++) begin
				mem[i] <= 8'b0;
			end
        else if(we) begin
            case (data_mask)
                /* sb */
                2'b00:  mem[alu_result]      <= wd[7:0];   

                /* sh */
                2'b01: begin
                         mem[alu_result]     <= wd[7:0]; 
                         mem[alu_result + 1] <= wd[15:8]; 
                end

                /* sw */
                2'b10: begin
                         mem[alu_result]     <= wd[7:0];
                         mem[alu_result + 1] <= wd[15:8];
                         mem[alu_result + 2] <= wd[23:16];
                         mem[alu_result + 3] <= wd[31:24];
                end
                
                /* default = byte */
                default: mem[alu_result] <= wd[7:0];
            endcase
        end
    end

    /* Read logic (combinational) */
    assign read_data = {mem[alu_result + 3], mem[alu_result + 2], mem[alu_result + 1], mem[alu_result]};
 endmodule

