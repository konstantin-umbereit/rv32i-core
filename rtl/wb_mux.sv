/* rtl/wb_max.sv
 *
 * Single-cycle RV32I writeback mux
 */

 module wb_mux #(
    parameter DATA_WIDTH = 32
 ) (
    input  logic [2:0]            result_src,   /* control from control unit */

    input  logic [DATA_WIDTH-1:0] alu_result,   /* output from alu */
    input  logic [DATA_WIDTH-1:0] result_data,  /* output from data memory */
    input  logic [DATA_WIDTH-1:0] imm_ext,      /* output from extender */
    input  logic [DATA_WIDTH-1:0] pc_target,    /* PC + imm_ext */
    input  logic [DATA_WIDTH-1:0] pc_plus4,     /* PC + 4 */

    output logic [DATA_WIDTH-1:0] result

 );

    always_comb begin
        case (result_src)
            3'b000: result = alu_result;
            3'b001: result = result_data;
            3'b010: result = imm_ext;
            3'b011: result = pc_target;
            3'b100: result = pc_plus4;
            
            default: result = alu_result;
        endcase
    end

 endmodule
