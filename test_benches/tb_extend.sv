/* test_benches/tb_extend.sv
 *
 * Test bench for rtl/extend.sv
 */
module tb_extend;

    logic [2:0] imm_src;
    logic [31:0] instr;

    logic [31:0] imm_ext;

    extend dut (.*);

    initial begin
        $dumpfile("waveforms/tb_extend.vcd");
        $dumpvars(0, tb_extend);

        #10 imm_src = 3'b000; instr = 32'h1234_5678; /* I-type, expected imm_ext = 0x00000123 */
        #10 imm_src = 3'b001; instr = 32'h1234_5678; /* S-type, expected imm_ext = 0x0000012C */
        #10 imm_src = 3'b010; instr = 32'h1234_5678; /* B-type, expected imm_ext = 0x00000096 */
        #10 imm_src = 3'b011; instr = 32'h1234_5678; /* U-type, expected imm_ext = 0x00012345  */
        #10 imm_src = 3'b100; instr = 32'h1234_5678; /* J-type, expected imm_ext = 0x00022C91 */

        #10 imm_src = 3'b101; instr = 32'h1234_5678; /* invalid imm_src, expected warning in simulator + imm_ext = 0 */
        #10 imm_src = 3'b110; instr = 32'h1234_5678; /* invalid imm_src, expected warning in simulator + imm_ext = 0 */
        #10 imm_src = 3'b111; instr = 32'h1234_5678; /* invalid imm_src, expected warning in simulator + imm_ext = 0 */


        #10 $finish;
    end

    initial $monitor("Time=0d%0t imm_src=0b%b instr=0x%h, imm_ext=0x%h", $time, imm_src, instr, imm_ext);

endmodule
