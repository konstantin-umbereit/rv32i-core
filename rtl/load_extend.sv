/* rtl/load_extend.sv
 *
 * Single-cycle RV32I load data extender
 */

 module load_extend #(
    parameter DATA_WIDTH = 32
 ) ( 
    input logic [DATA_WIDTH-1:0] load_data,
    input logic [2:0]            data_mask,
    output logic [DATA_WIDTH-1:0] load_data_ext
 );

 always_comb begin
    case (data_mask)
        3'b000: load_data_ext = {{24{load_data[7]}}, load_data[7:0]};   /* lb */
        3'b001: load_data_ext = {{16{load_data[15]}}, load_data[15:0]}; /* lh */
        3'b010: load_data_ext = load_data;                              /* lw */
        3'b100: load_data_ext = {24'b0, load_data[7:0]};                /* lbu */
        3'b101: load_data_ext = {16'b0, load_data[15:0]};               /* lhu */
        default: load_data_ext = {{24{load_data[7]}}, load_data[7:0]};  /* default */
    endcase
 end

 endmodule
