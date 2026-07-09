/* rtl/alu.sv
 *
 * Single-cycle RV32I arithmetic logic unit
 */

 module alu #(
    parameter DATA_WIDTH = 32
 ) (
    input  logic [3:0]            alu_ctrl,                /* from conrol unit */
    
    input  logic [DATA_WIDTH-1:0] src_a,                   /* rd1 from regfile */
    input  logic [DATA_WIDTH-1:0] src_b,                   /* rd2 fromregfile or imm_ext from extend */
    
    output logic [DATA_WIDTH-1:0] alu_result,

    output logic                  zero                     /* for branch condition */
 );
    always_comb begin
        case (alu_ctrl) 

            /* ADD: add, addi, lw, sw, auipic, jal, jalr */
            4'b0000 : begin 
                alu_result[DATA_WIDTH-1:0] = src_a[DATA_WIDTH-1:0] + src_b[DATA_WIDTH-1:0];
            end

            /* SUB: sub, beq, bne, bit, bge */
            4'b0001 : begin 
                alu_result[DATA_WIDTH-1:0] = src_a[DATA_WIDTH-1:0] - src_b[DATA_WIDTH-1:0];
            end

            /* AND: and, andi */
            4'b0010 : begin 
                alu_result[DATA_WIDTH-1:0] = src_a[DATA_WIDTH-1:0] & src_b[DATA_WIDTH-1:0];
            end

            /* OR: or, ori */
            4'b0011 : begin 
                alu_result[DATA_WIDTH-1:0] = src_a[DATA_WIDTH-1:0] | src_b[DATA_WIDTH-1:0];
            end

            /* XOR: xor, xor */
            4'b0100 : begin
                alu_result[DATA_WIDTH-1:0] = src_a[DATA_WIDTH-1:0] ^ src_b[DATA_WIDTH-1:0];
            end

            /* SLT: slt */
            4'b0101 : begin
                alu_result[DATA_WIDTH-1:0]  = ($signed(src_a) < $signed(src_b)) ? 32'b1: 32'b0;
            end

            /* SLTU: sltu, sltiu */
            4'b0110 : begin
                alu_result[DATA_WIDTH-1:0]  = ($unsigned(src_a) < $unsigned(src_b)) ? 32'b1: 32'b0;
            end

            /* SLL: sll, slli */
            4'b0111 : begin
                alu_result[DATA_WIDTH-1:0]  = src_a[DATA_WIDTH-1:0]  << src_b[4:0];
            end

            /* SRL: srl, srli*/
            4'b1000 : begin
                alu_result[DATA_WIDTH-1:0]  = src_a[DATA_WIDTH-1:0]  >> src_b[4:0]; 
            end

            /* SRA: sra, srai */
            4'b1001 : begin
                alu_result[DATA_WIDTH-1:0]  = $signed(src_a[DATA_WIDTH-1:0])  >>> src_b[4:0];
            end
            
            default: begin
                alu_result = 32'b0;
                $display("alu module received invalid  alu_ctrl=0b%b at time=0d%t", alu_ctrl, $time);
            end
        endcase
    end

    assign zero = (alu_result == 32'b0);

 endmodule
