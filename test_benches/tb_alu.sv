/* test_benches/tb_alu.sv
 *
 * Test bench for rtl/alu.sv
 */

 module tb_alu;
    logic [3:0] alu_ctrl;
    logic [31:0] src_a, src_b;
    logic [31:0] alu_result;
    logic zero;

    alu dut (.*);

    initial begin
        $dumpfile("waveforms/tb_alu.vcd");
        $dumpvars(0, tb_alu);

        #10 alu_ctrl = 4'b0000; src_a = 32'h0000_0003; src_b = 32'h0000_0007; /* ADD,  expected alu_result=0x0000000a +  zero=0  */
        #10 alu_ctrl = 4'b0001; src_a = 32'h0000_0003; src_b = 32'h0000_0007; /* SUB,  expected alu_result=0xfffffffc +  zero=0  */ 
        #10 alu_ctrl = 4'b0010; src_a = 32'h0000_0003; src_b = 32'h0000_0007; /* AND,  expected alu_result=0x00000003 +  zero=0  */
        #10 alu_ctrl = 4'b0011; src_a = 32'h0000_0003; src_b = 32'h0000_0007; /* OR,   expected alu_result=0x00000007 +  zero=0  */
        #10 alu_ctrl = 4'b0100; src_a = 32'h0000_0003; src_b = 32'h0000_0007; /* XOR,  expected alu_result=0x00000004 +  zero=0  */
        #10 alu_ctrl = 4'b0101; src_a = 32'h0000_0003; src_b = 32'h0000_0007; /* SLT,  expected alu_result=0x00000001 +  zero=0  */
        #10 alu_ctrl = 4'b0110; src_a = 32'h0000_0003; src_b = 32'h0000_0007; /* SLTU, expected alu_result=0x00000001 +  zero=0  */
        #10 alu_ctrl = 4'b0111; src_a = 32'h0000_0003; src_b = 32'h0000_0007; /* SLL,  expected alu_result=0x00000180 +  zero=0  */
        #10 alu_ctrl = 4'b1000; src_a = 32'h0000_0003; src_b = 32'h0000_0007; /* SRL,  expected alu_result=0x00000000 +  zero=1  */
        #10 alu_ctrl = 4'b1001; src_a = 32'h0000_0003; src_b = 32'h0000_0007; /* SRA.  expected alu_result=0x00000000 +  zero=1  */
        #10 $finish;

    end

    initial $monitor("Time=0d%0t, alu_ctrl=0b%b, src_a=0x%h, src_b=0x%h, alu_result=0x%h, zero=%b", $time, alu_ctrl, src_a, src_b, alu_result, zero);
 endmodule
