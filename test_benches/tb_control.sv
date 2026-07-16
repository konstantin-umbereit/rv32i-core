/* test_benches/tb_control.sv
 *
 * Test bench for rtl/control.sv
 */

 module tb_control;

    logic [6:0] op;
    logic [2:0] funct3;
    logic [6:0] funct7;
    logic       zero;

    logic [1:0] pc_src;      
    logic [2:0] result_src;  
    logic       mem_write;   
    logic       alu_src;     
    logic [2:0] imm_src;     
    logic       reg_write;   

    logic [3:0] alu_ctrl;

    control dut (.*);

    initial begin
        $dumpfile("waveforms/tb_control.vcd");
        $dumpvars(0, tb_control);

        #10 op = 7'b0110011; funct3 = 3'b000; funct7 = 7'b0000000; zero = 0; /* R-type add */
        #10 op = 7'b0010011; funct3 = 3'b000; funct7 = 7'b0000000; zero = 0; /* I-type addi */
        #10 op = 7'b0000011; funct3 = 3'b010; funct7 = 7'b0000000; zero = 0; /* I-type lw */
        #10 op = 7'b0100011; funct3 = 3'b010; funct7 = 7'b0000000; zero = 0; /* S-type sw */
        #10 op = 7'b1100011; funct3 = 3'b000; funct7 = 7'b0000000; zero = 1; /* B-type beq (taken)*/
        #10 op = 7'b1100011; funct3 = 3'b000; funct7 = 7'b0000000; zero = 0; /* B-type beq (not taken) */
        #10 op = 7'b0110111; funct3 = 3'b000; funct7 = 7'b0000000; zero = 0; /* U-type lui */
        #10 op = 7'b0010111; funct3 = 3'b000; funct7 = 7'b0000000; zero = 0; /* U-type auipc */
        #10 op = 7'b1101111; funct3 = 3'b000; funct7 = 7'b0000000; zero = 0; /* J-type jal */
        #10 op = 7'b1100111; funct3 = 3'b000; funct7 = 7'b0000000; zero = 0; /* I-type jalr */
        

        #10 $finish;
    end

    initial $monitor("Time=0d%0t op=0b%b funct3=0b%b funct7=0b%b zero=0b%b | pc_src=0b%b  result_src=0b%b mem_write=0b%b  alu_src=0b%b imm_src=0b%b reg_write=0b%b alu_ctrl=0b%b",
                     $time, op, funct3, funct7, zero, pc_src, result_src, mem_write, alu_src, imm_src, reg_write, alu_ctrl);

endmodule
