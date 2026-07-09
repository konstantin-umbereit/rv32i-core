/* rtl/tb_extend.sv
 *
 * test bench for rtl/tb_extend.sv
 */
module tb_extend;

    logic [2:0] imm_src;
    logic [31:0] instr;

    logic [31:0] imm_ext;

    extend dut (.*);

    initial begin
        $dumpfile("waveforms/tb_extend.vcd");
        $dumpvars(0, tb_extend);

        #10 imm_src = 3'b000; instr = 32'h1234_5678; /* I-type, imm = 0x123*/
        #10 imm_src = 3'b001; instr = 32'h1234_5678; /* S-type, imm = 0x12C */
        #10 imm_src = 3'b010; instr = 32'h1234_5678; /* B-type, imm = 0x96*/
        #10 imm_src = 3'b011; instr = 32'h1234_5678; /* U-type, imm = 0x12345*/
        #10 imm_src = 3'b100; instr = 32'h1234_5678; /* J-type, imm = 0x22C91*/

        #10 imm_src = 3'b101; instr = 32'h1234_5678; /* invalid imm_src, expected warning in simulator + imm = 0 */
        #10 imm_src = 3'b110; instr = 32'h1234_5678; /* invalid imm_src, expected warning in simulator + imm = 0 */
        #10 imm_src = 3'b111; instr = 32'h1234_5678; /* invalid imm_src, expected warning in simulator + imm = 0 */


        #10 $finish;
    end

    initial $monitor("Time=0d%0t imm_src=0b%b instr=0x%h, imm_ext=0x%h", $time, imm_src, instr, imm_ext);

endmodule
