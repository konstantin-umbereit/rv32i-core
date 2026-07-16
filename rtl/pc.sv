/* rtl/pc.sv
 *
 * Single-cycle RV32I program counter
 */
module pc #(
    parameter DATA_WIDTH = 32
) (
    input  logic clk,                       /* rising edge */
    input  logic rst_n,                     /* active-low asynchronous reset */
    input logic  halt,                      /* signal from control unit: halts the program counter */

    input  logic [DATA_WIDTH-1:0] pc_next,   /* next PC (either pc+4 or target) */
    output logic [DATA_WIDTH-1:0] pc_out    /* program counter output */
);
    /* Single 32-bit wide program counter register */
    logic [DATA_WIDTH-1:0] pc;

    /* Write logic */
    always_ff @(posedge clk or negedge rst_n) begin
        if(!rst_n) begin
            pc <= 32'b0;
        end 
        else if(!halt) begin
            pc <= pc_next;
        end
    end
    
    assign pc_out = pc;

endmodule
