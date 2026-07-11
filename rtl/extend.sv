/* rtl/extend.sv
 *
 * Single-cycle RV32I sign extender
 */

 module extend #(
    parameter DATA_WIDTH = 32
 ) (
    input  logic [2:0]             imm_src, /* comes directly from the control unit */

    input  logic [DATA_WIDTH-1:0]  instr,
    output logic [DATA_WIDTH-1:0]  imm_ext
 );

    always_comb begin
        case (imm_src)
            /* I-type */
            3'b000: begin
                imm_ext[31:12] = {20{instr[31]}};
                imm_ext[11:0]  = instr[31:20]; 
            end 

            /* S-type */
            3'b001: begin
                imm_ext[31:12] = {20{instr[31]}};
                imm_ext[11:5]  = instr[31:25]; 
                imm_ext[4:0]   = instr[11:7];
            end

            /* B-type */
            3'b010: begin
                imm_ext[31:13] = {19{instr[31]}};
                imm_ext[12]    = instr[31];
                imm_ext[11]    = instr[7];
                imm_ext[10:5]  = instr[30:25];
                imm_ext[4:1]   = instr[11:8];
                imm_ext[0]     = 1'b0;
                
            end

            /* U-type */
            3'b011: begin
                imm_ext[31:12] = instr[31:12];
                imm_ext[11:0]  = 12'b0;
            end

            /* J-type */
            3'b100: begin
                imm_ext[31:21] = {11{instr[31]}};
                imm_ext[20]    = instr[31];
                imm_ext[19:12] = instr[19:12];
                imm_ext[11]    = instr[20];
                imm_ext[10:1]  = instr[30:21];
                imm_ext[0]     = 1'b0;
            end
            default: begin
                imm_ext[DATA_WIDTH-1:0] = 32'b0;
                $display("WARNING: extend module received invalid imm_src=0b%b at time=0d%0t", imm_src, $time);
            end

        endcase
    end

endmodule
